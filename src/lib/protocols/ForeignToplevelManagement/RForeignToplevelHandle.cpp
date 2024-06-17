#include <protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <protocols/ForeignToplevelManagement/wlr-foreign-toplevel-management-unstable-v1.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LFactory.h>
#include <LClient.h>

using namespace Louvre::Protocols::ForeignToplevelManagement;

static const struct zwlr_foreign_toplevel_handle_v1_interface imp
{
    .set_maximized = &RForeignToplevelHandle::set_maximized,
    .unset_maximized = &RForeignToplevelHandle::unset_maximized,
    .set_minimized = &RForeignToplevelHandle::set_minimized,
    .unset_minimized = &RForeignToplevelHandle::unset_minimized,
    .activate = &RForeignToplevelHandle::activate,
    .close = &RForeignToplevelHandle::close,
    .set_rectangle = &RForeignToplevelHandle::set_rectangle,
    .destroy = &RForeignToplevelHandle::destroy,
#if LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION >= 2
    .set_fullscreen = &RForeignToplevelHandle::set_fullscreen,
    .unset_fullscreen = &RForeignToplevelHandle::unset_fullscreen
#endif
};

RForeignToplevelHandle::RForeignToplevelHandle
    (
        GForeignToplevelManager &foreignToplevelManagerRes,
        LToplevelRole &toplevelRole
    )
    :LResource
    (
        foreignToplevelManagerRes.client(),
        &zwlr_foreign_toplevel_handle_v1_interface,
        foreignToplevelManagerRes.version(),
        0,
        &imp
        ),
    m_foreignToplevelManagerRes(&foreignToplevelManagerRes),
    m_toplevelRole(&toplevelRole),
    m_controller(LFactory::createObject<LForeignToplevelController>(this))
{
    zwlr_foreign_toplevel_manager_v1_send_toplevel(foreignToplevelManagerRes.resource(), resource());

    m_toplevelRole.setOnDestroyCallback([this](auto) {
        closed();
    });

    title(toplevelRole.title());
    appId(toplevelRole.appId());

    for (LOutput *o : toplevelRole.surface()->outputs())
        outputEnter(o);

    updateState();

    if (toplevelRole.surface()->parent() && toplevelRole.surface()->parent()->toplevel())
    {
        for (auto *controller : toplevelRole.surface()->parent()->toplevel()->m_foreignControllers)
        {
            if (controller->resource().foreignToplevelManagerRes() == &foreignToplevelManagerRes)
            {
                parent(&controller->m_resource);
                break;
            }
        }
    }

    done();
}

RForeignToplevelHandle::~RForeignToplevelHandle()
{
    compositor()->onAnticipatedObjectDestruction(m_controller.get());
}

void RForeignToplevelHandle::updateState() noexcept
{
    if (!toplevelRole())
        return;

    zwlr_foreign_toplevel_handle_v1_state states[4];

    wl_array arr
    {
        .size = 0,
        .alloc = 0,
        .data = &states,
    };

    if (toplevelRole()->surface()->minimized())
        states[arr.size++] = ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED;
    if (toplevelRole()->maximized())
        states[arr.size++] = ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED;
    if (toplevelRole()->activated())
        states[arr.size++] = ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED;
    if (version() >= 2 && toplevelRole()->fullscreen())
        states[arr.size++] = ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN;

    arr.size *= sizeof(states[0]);
    arr.alloc = arr.size;
    state(&arr);
}

/******************** REQUESTS ********************/

void RForeignToplevelHandle::set_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::MaximizeCap))
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->setMaximizedRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::unset_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::MaximizeCap))
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->unsetMaximizedRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::set_minimized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::MinimizeCap))
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->setMinimizedRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::unset_minimized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::MinimizeCap))
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->unsetMinimizedRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::activate(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped() || !res.m_toplevelRole)
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->activateRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::close(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped() || !res.m_toplevelRole)
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->closeRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::set_rectangle(wl_client */*client*/, wl_resource *resource, wl_resource *surface, Int32 x, Int32 y, Int32 width, Int32 height)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || !res.m_toplevelRole || res.m_foreignToplevelManagerRes->stopped())
        return;

    if (width < 0 || height < 0)
    {
        wl_resource_post_error(resource, ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_ERROR_INVALID_RECTANGLE, "The provided rectangle is invalid.");
        return;
    }

    if (width == 0 && height == 0)
    {
        res.controller()->m_taskbarIconRect = LRect();

        if (res.controller()->taskbar())
        {
            res.controller()->m_taskbar.reset();
            res.controller()->taskbarChanged();
        }

        return;
    }

    LSurface *lsurface { static_cast<Protocols::Wayland::RSurface*>(wl_resource_get_user_data(surface))->surface() };
    const LRect newRect { x, y, width, height };

    if (res.controller()->m_taskbarIconRect != newRect || lsurface != res.controller()->taskbar())
    {
        res.controller()->m_taskbarIconRect = newRect;
        res.controller()->m_taskbar = lsurface;
        res.controller()->taskbarChanged();
    }
}

void RForeignToplevelHandle::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

#if LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION >= 2
void RForeignToplevelHandle::set_fullscreen(wl_client */*client*/, wl_resource *resource, wl_resource *output)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::FullscreenCap))
        return;

    LOutput *dstOutput { nullptr };

    if (output)
        dstOutput = static_cast<Protocols::Wayland::GOutput*>(wl_resource_get_user_data(output))->output();

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->setFullscreenRequest(dstOutput);
    res.toplevelRole()->m_requesterController = nullptr;
}

void RForeignToplevelHandle::unset_fullscreen(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RForeignToplevelHandle*>(wl_resource_get_user_data(resource)) };

    if (!res.m_foreignToplevelManagerRes || res.m_foreignToplevelManagerRes->stopped()
        || !res.m_toplevelRole || !res.m_toplevelRole->capabilities().check(LToplevelRole::FullscreenCap))
        return;

    res.toplevelRole()->m_requesterController = res.controller();
    res.toplevelRole()->unsetFullscreenRequest();
    res.toplevelRole()->m_requesterController = nullptr;
}
#endif

/******************** EVENTS ********************/

void RForeignToplevelHandle::title(const std::string &title) noexcept
{
    m_changed = true;
    zwlr_foreign_toplevel_handle_v1_send_title(resource(), title.c_str());
}

void RForeignToplevelHandle::appId(const std::string &appId) noexcept
{
    m_changed = true;
    zwlr_foreign_toplevel_handle_v1_send_app_id(resource(), appId.c_str());
}

void RForeignToplevelHandle::outputEnter(LOutput *output) noexcept
{
    for (auto *outputRes : client()->outputGlobals())
        if (outputRes->output() == output)
            zwlr_foreign_toplevel_handle_v1_send_output_enter(resource(), outputRes->resource());
}

void RForeignToplevelHandle::outputLeave(LOutput *output) noexcept
{
    for (auto *outputRes : client()->outputGlobals())
        if (outputRes->output() == output)
            zwlr_foreign_toplevel_handle_v1_send_output_leave(resource(), outputRes->resource());
}

void RForeignToplevelHandle::state(wl_array *state) noexcept
{
    m_changed = true;
    zwlr_foreign_toplevel_handle_v1_send_state(resource(), state);
}

void RForeignToplevelHandle::done() noexcept
{
    m_changed = false;
    zwlr_foreign_toplevel_handle_v1_send_done(resource());
}

void RForeignToplevelHandle::closed() noexcept
{
    zwlr_foreign_toplevel_handle_v1_send_closed(resource());
}

bool RForeignToplevelHandle::parent(RForeignToplevelHandle *parent) noexcept
{
#if LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        m_changed = true;
        zwlr_foreign_toplevel_handle_v1_send_parent(resource(), parent == nullptr ? nullptr : parent->resource());
        return true;
    }
#endif
    L_UNUSED(parent)
    return false;
}
