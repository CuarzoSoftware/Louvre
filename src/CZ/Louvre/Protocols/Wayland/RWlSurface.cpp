#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/TearingControl/RTearingControl.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Protocols/Wayland/GCompositor.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/CZCore.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_surface_interface imp =
{
    .destroy                = &RWlSurface::destroy,
    .attach                 = &RWlSurface::attach,
    .damage                 = &RWlSurface::damage,
    .frame                  = &RWlSurface::frame,
    .set_opaque_region      = &RWlSurface::set_opaque_region,
    .set_input_region       = &RWlSurface::set_input_region,
    .commit                 = &RWlSurface::commit,

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &RWlSurface::set_buffer_transform,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &RWlSurface::set_buffer_scale,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &RWlSurface::damage_buffer,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    .offset                 = &RWlSurface::offset
#endif
};

RWlSurface::RWlSurface
(
    GCompositor *compositorRes,
    UInt32 id
)
    :LResource
    (
        compositorRes->client(),
        &wl_surface_interface,
        compositorRes->version(),
        id,
        &imp
    )
{
    // Create surface
    LSurface::Params params;
    params.surfaceResource = this;
    m_surface.reset(LFactory::createObject<LSurface>(&params));
}

RWlSurface::~RWlSurface()
{
    LSurface *lSurface { this->surface() };
    lSurface->imp()->destroyCursorOrDNDRole();

    assert(lSurface->imp()->canHostRole());

    lSurface->imp()->setKeyboardGrabToParent();

    // Notify from client
    compositor()->onAnticipatedObjectDestruction(lSurface);
    compositor()->onAnticipatedObjectDestruction(lSurface->imp()->backgroundBlur.get());

    lSurface->imp()->setMapped(false);
    lSurface->imp()->stateFlags.add(LSurface::LSurfacePrivate::Destroyed);
}

/******************** REQUESTS ********************/

using Changes = LSurfaceCommitEvent::Changes;

void RWlSurface::handleOffset(LSurface *surface, Int32 x, Int32 y)
{
    if (surface->role())
        surface->role()->handleSurfaceOffset(x, y);
}

void RWlSurface::attach(wl_client */*client*/, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y)
{
    auto &surfaceRes { *static_cast<RWlSurface*>(wl_resource_get_user_data(resource)) };

    surfaceRes.surface()->imp()->pending.buffer.attached = true;
    surfaceRes.surface()->imp()->pending.buffer.released = false;
    surfaceRes.surface()->imp()->pending.buffer.signaled = false;

    if (surfaceRes.surface()->role())
        surfaceRes.surface()->role()->handleSurfaceBufferAttach(buffer, x, y);

    surfaceRes.surface()->imp()->pending.buffer.buffer.setResource(buffer);

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    if (surfaceRes.version() < 5)
        handleOffset(surfaceRes.surface(), x, y);
    else
        if (x != 0 || y != 0)
            surfaceRes.postError(WL_SURFACE_ERROR_INVALID_OFFSET, "Buffer offset is invalid. Check wl_surface::offset (v5).");
#else
    handleOffset(surfaceRes.surface(), x, y);
#endif
}

void RWlSurface::frame(wl_client *client, wl_resource *resource, UInt32 callback)
{
    auto &imp { *static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
    imp.pending.frames.resources.emplace_back(wl_resource_create(client, &wl_callback_interface, 1, callback));
}

void RWlSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &surfaceRes { *static_cast<RWlSurface*>(wl_resource_get_user_data(resource)) };

    surfaceRes.surface()->imp()->destroyCursorOrDNDRole();

    if (!surfaceRes.surface()->imp()->canHostRole())
    {
        surfaceRes.postError(WL_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT, "Surface destroyed before role.");
        return;
    }

    wl_resource_destroy(resource);
}

void RWlSurface::commit(wl_client */*client*/, wl_resource *resource)
{
    static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp()->handleCommit();
}

void RWlSurface::damage(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    auto &imp { *static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    // Ignore rects with invalid or crazy sizes
    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    imp.pending.damage.emplace_back(SkIRect::MakeXYWH(x, y, width, height));
    imp.pending.changesToNotify.add(Changes::DamageRegionChanged);
}

void RWlSurface::set_opaque_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &imp { *static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    if (region)
    {
        imp.pending.opaqueRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
        imp.pending.changesToNotify.add(Changes::OpaqueRegionChanged);
    }
    else if (!imp.pending.opaqueRegion.isEmpty())
    {
        imp.pending.opaqueRegion.setEmpty();
        imp.pending.changesToNotify.add(Changes::OpaqueRegionChanged);
    }
}

void RWlSurface::set_input_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &imp { *static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    if (region)
    {
        imp.pending.inputRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
        imp.stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInput);
        imp.pending.changesToNotify.add(Changes::InputRegionChanged);
    }
    else if (!imp.stateFlags.has(LSurface::LSurfacePrivate::InfiniteInput))
    {
        imp.pending.inputRegion.setEmpty();
        imp.stateFlags.add(LSurface::LSurfacePrivate::InfiniteInput);
        imp.pending.changesToNotify.add(Changes::InputRegionChanged);
    }
}

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
void RWlSurface::set_buffer_transform(wl_client */*client*/, wl_resource *resource, Int32 transform)
{
    auto &res { *static_cast<RWlSurface*>(wl_resource_get_user_data(resource)) };
    if (transform < 0 || transform > 7)
    {
        res.postError(WL_SURFACE_ERROR_INVALID_TRANSFORM, "Invalid framebuffer transform %d.", transform);
        return;
    }

    auto &imp { *res.surface()->imp() };
    imp.pending.transform = static_cast<CZTransform>(transform);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
void RWlSurface::set_buffer_scale(wl_client */*client*/, wl_resource *resource, Int32 scale)
{
    auto &res { *static_cast<RWlSurface*>(wl_resource_get_user_data(resource)) };

    if (scale <= 0)
    {
        res.postError(WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    auto &imp { *res.surface()->imp() };
    imp.pending.scale = scale;
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
void RWlSurface::damage_buffer(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    // Ignore rects with invalid or crazy sizes

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    auto &imp { *static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
    imp.pending.bufferDamage.emplace_back(SkIRect::MakeXYWH(x, y, width, height));
    imp.pending.changesToNotify.add(Changes::DamageRegionChanged);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
void RWlSurface::offset(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y)
{
    auto *surf { static_cast<const RWlSurface*>(wl_resource_get_user_data(resource))->surface() };
    handleOffset(surf, x, y);
}
#endif

/******************** EVENTS ********************/

void RWlSurface::enter(GOutput *outputRes) noexcept
{
    wl_surface_send_enter(resource(), outputRes->resource());
}

void RWlSurface::leave(GOutput *outputRes) noexcept
{
    wl_surface_send_leave(resource(), outputRes->resource());
}

bool RWlSurface::preferredBufferScale(Int32 scale) noexcept
{
#if LOUVRE_WL_COMPOSITOR_VERSION >= 6
    if (version() >= 6)
    {
        wl_surface_send_preferred_buffer_scale(resource(), scale);
        return true;
    }
#endif
    L_UNUSED(scale);
    return false;
}

bool RWlSurface::preferredBufferTransform(CZTransform transform) noexcept
{
#if LOUVRE_WL_COMPOSITOR_VERSION >= 6
    if (version() >= 6)
    {
        wl_surface_send_preferred_buffer_transform(resource(), static_cast<UInt32>(transform));
        return true;
    }
#endif
    L_UNUSED(transform);
    return false;
}
