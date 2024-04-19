#include <protocols/XdgShell/GXdgWmBase.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/RXdgPositioner.h>
#include <protocols/XdgShell/RXdgPopup.h>
#include <private/LSurfacePrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LPopupRolePrivate.h>
#include <LSubsurfaceRole.h>

using namespace Louvre::Protocols::XdgShell;

static const struct xdg_surface_interface imp
{
    .destroy = &RXdgSurface::destroy,
    .get_toplevel = &RXdgSurface::get_toplevel,
    .get_popup = &RXdgSurface::get_popup,
    .set_window_geometry = &RXdgSurface::set_window_geometry,
    .ack_configure = &RXdgSurface::ack_configure
};

RXdgSurface::RXdgSurface
(
    GXdgWmBase *xdgWmBaseRes,
    LSurface *surface,
    UInt32 id
) noexcept
    :LResource
    (
        xdgWmBaseRes->client(),
        &xdg_surface_interface,
        xdgWmBaseRes->version(),
        id,
        &imp
    ),
    m_xdgWmBaseRes(xdgWmBaseRes),
    m_surface(surface)
{
    m_xdgWmBaseRes->m_xdgSurfacesCount++;
}

RXdgSurface::~RXdgSurface() noexcept
{
    if (xdgWmBaseRes())
        xdgWmBaseRes()->m_xdgSurfacesCount--;
}

/******************** REQUESTS ********************/

LRect RXdgSurface::calculateGeometryWithSubsurfaces() noexcept
{
    if (!surface())
        return LRect(0,0,1,1);

    LRegion region { LRect(0, 0, surface()->size().w(), surface()->size().h()) };

    for (LSurface *child : surface()->children())
        if (child->subsurface() && child->mapped())
            region.addRect(child->subsurface()->localPos(), child->size());

    return LRect(
        region.extents().x1,
        region.extents().y1,
        region.extents().x2 - region.extents().x1,
        region.extents().y2 - region.extents().y1
    );
}

void RXdgSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (res.surface() && res.surface()->role())
    {
        // TODO: Update XML and replace by DEFUNC_ROLE_OBJECT error
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "xdg_surface must be destroyed after its specific role");
        return;
    }

    wl_resource_destroy(resource);
}

void RXdgSurface::get_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    auto *res { static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (!res->surface() || res->surface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (res->surface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    new RXdgToplevel(res, id);
}

void RXdgSurface::get_popup(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner)
{
    if (!parent)
    {
        wl_resource_post_error(positioner, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT, "xdg_popup's without parent not supported");
        return;
    }

    auto *xdgPositionerRes { static_cast<RXdgPositioner*>(wl_resource_get_user_data(positioner)) };

    if (!xdgPositionerRes->validate())
        return;

    auto *res { static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (res->surface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (res->surface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    auto *xdgParentSurfaceRes { static_cast<RXdgSurface*>(wl_resource_get_user_data(parent)) };

    if (res->surface()->imp()->isInChildrenOrPendingChildren(xdgParentSurfaceRes->surface()))
    {
        wl_resource_post_error(positioner,
                               XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "Parent can not be child or equal to surface.");
        return;
    }

    new RXdgPopup(res, xdgParentSurfaceRes, xdgPositionerRes, id);
}

void RXdgSurface::set_window_geometry(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    auto &res { *static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (!res.xdgPopupRes() && !res.xdgToplevelRes())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "Can not set window geometry with no role.");
        return;
    }

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_INVALID_SIZE, "Invalid window geometry size.");
        return;
    }

    res.m_pendingWindowGeometry.setX(x);
    res.m_pendingWindowGeometry.setY(y);
    res.m_pendingWindowGeometry.setW(width);
    res.m_pendingWindowGeometry.setH(height);
    res.m_windowGeometrySet = true;
    res.m_hasPendingWindowGeometry = true;
}

void RXdgSurface::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &res { *static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (!res.surface() || (!res.xdgPopupRes() && !res.xdgToplevelRes()))
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "Can not ack xdg_surface with no role.");
        return;
    }

    if (res.surface()->roleId() == LSurface::Role::Toplevel)
    {
        auto &toplevel { *res.surface()->toplevel() };

        while (!toplevel.imp()->sentConfs.empty())
        {
            if (toplevel.imp()->sentConfs.front().serial == serial)
            {
                toplevel.imp()->stateFlags.add(LToplevelRole::LToplevelRolePrivate::HasUncommitedConfiguration);
                toplevel.imp()->uncommited = toplevel.imp()->sentConfs.front();

                if (!toplevel.imp()->xdgDecoration)
                    toplevel.imp()->uncommited.decorationMode = LToplevelRole::ClientSide;

                toplevel.imp()->sentConfs.pop_front();

                if (toplevel.imp()->uncommited.size == toplevel.imp()->current.size)
                    toplevel.imp()->applyPendingChanges(0);
                return;
            }

            toplevel.imp()->sentConfs.pop_front();
        }

        wl_resource_post_error(res.resource(), XDG_SURFACE_ERROR_INVALID_SERIAL, "Invalid xdg_surface serial ack.");
    }
    else if (res.surface()->roleId() == LSurface::Role::Popup)
    {
        auto &popup { *res.surface()->popup() };

        while (!popup.imp()->sentConfs.empty())
        {
            if (popup.imp()->sentConfs.front().serial == serial)
            {
                popup.imp()->previous = popup.imp()->current;
                popup.imp()->current = popup.imp()->sentConfs.front();
                popup.imp()->sentConfs.pop_front();
                popup.configurationChanged();
                return;
            }

            popup.imp()->sentConfs.pop_front();
        }

        wl_resource_post_error(res.resource(), XDG_SURFACE_ERROR_INVALID_SERIAL, "Invalid xdg_surface serial ack.");
    }
    else
        wl_resource_post_error(res.resource(), XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "wl_surface does not have a role yet.");
}

/******************** EVENTS ********************/

void RXdgSurface::configure(UInt32 serial) noexcept
{
    xdg_surface_send_configure(resource(), serial);
}
