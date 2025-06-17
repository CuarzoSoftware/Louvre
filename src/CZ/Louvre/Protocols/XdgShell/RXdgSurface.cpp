#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgToplevel.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPositioner.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPopup.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>

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

SkIRect RXdgSurface::calculateGeometryWithSubsurfaces() noexcept
{
    if (!surface())
        return SkIRect::MakeWH(1, 1);

    SkRegion region { SkIRect::MakeWH(surface()->size().width(), surface()->size().height()) };

    for (LSurface *child : surface()->children())
        if (child->subsurface() && child->mapped())
            region.op(SkIRect::MakePtSize(child->subsurface()->localPos(), child->size()), SkRegion::Op::kUnion_Op);

    return region.getBounds();
}

void RXdgSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (res.surface() && res.surface()->role())
    {
        res.postError(XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "xdg_surface must be destroyed after its specific role");
        return;
    }

    wl_resource_destroy(resource);
}

void RXdgSurface::get_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    auto *res { static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (!res->surface() || res->surface()->imp()->hasBufferOrPendingBuffer())
    {
        res->postError(XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (res->surface()->role())
    {
        res->postError(XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    new RXdgToplevel(res, id);
}

void RXdgSurface::get_popup(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner)
{
    auto *xdgPositionerRes { static_cast<RXdgPositioner*>(wl_resource_get_user_data(positioner)) };

    if (!xdgPositionerRes->validate())
        return;

    auto *res { static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (res->surface()->imp()->hasBufferOrPendingBuffer())
    {
        res->postError(XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (res->surface()->role())
    {
        res->postError(XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    auto *xdgParentSurfaceRes { parent == nullptr ? nullptr : static_cast<RXdgSurface*>(wl_resource_get_user_data(parent)) };

    if (xdgParentSurfaceRes && res->surface()->imp()->isInChildrenOrPendingChildren(xdgParentSurfaceRes->surface()))
    {
        xdgPositionerRes->postError(
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
        res.postError(XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "Can not set window geometry with no role.");
        return;
    }

    if (width <= 0 || height <= 0)
    {
        res.postError(XDG_SURFACE_ERROR_INVALID_SIZE, "Invalid window geometry size.");
        return;
    }

    res.m_pendingWindowGeometry.setXYWH(x, y, width, height);
    res.m_windowGeometrySet = true;
    res.m_hasPendingWindowGeometry = true;
}

void RXdgSurface::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &res { *static_cast<RXdgSurface*>(wl_resource_get_user_data(resource)) };

    if (!res.surface() || (!res.xdgPopupRes() && !res.xdgToplevelRes()))
    {
        res.postError(XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "Can not ack xdg_surface with no role.");
        return;
    }

    if (res.surface()->roleId() == LSurface::Role::Toplevel)
    {
        auto &toplevel { *res.surface()->toplevel() };

        while (!toplevel.m_sentConfigurations.empty())
        {
            if (toplevel.m_sentConfigurations.front().serial == serial)
            {
                const bool sizeUnchanged { toplevel.m_lastACKConfiguration.size == toplevel.m_sentConfigurations.front().size };

                toplevel.m_lastACKConfiguration = toplevel.m_sentConfigurations.front();
                toplevel.m_sentConfigurations.pop_front();

                toplevel.pendingAtoms().serial = toplevel.m_lastACKConfiguration.serial;
                toplevel.pendingAtoms().bounds = toplevel.m_lastACKConfiguration.bounds;
                toplevel.pendingAtoms().capabilities = toplevel.m_lastACKConfiguration.capabilities;
                toplevel.pendingAtoms().decorationMode = toplevel.m_lastACKConfiguration.decorationMode;
                toplevel.pendingAtoms().state = toplevel.m_lastACKConfiguration.state;

                if (sizeUnchanged && !res.m_hasPendingWindowGeometry)
                    toplevel.fullAtomsUpdate();
                else
                    toplevel.partialAtomsUpdate();

                return;
            }

            toplevel.m_sentConfigurations.pop_front();
        }

        res.postError(XDG_SURFACE_ERROR_INVALID_SERIAL, "Invalid xdg_surface serial ack.");
    }
    else if (res.surface()->roleId() == LSurface::Role::Popup)
    {
        auto &popup { *res.surface()->popup() };

        while (!popup.m_sentConfs.empty())
        {
            if (popup.m_sentConfs.front().serial == serial)
            {
                popup.m_lastACKConfiguration = popup.m_sentConfs.front();
                popup.m_sentConfs.pop_front();
                return;
            }

            popup.m_sentConfs.pop_front();
        }

        res.postError(XDG_SURFACE_ERROR_INVALID_SERIAL, "Invalid xdg_surface serial ack.");
    }
    else
        res.postError(XDG_SURFACE_ERROR_NOT_CONSTRUCTED, "wl_surface does not have a role yet.");
}

/******************** EVENTS ********************/

void RXdgSurface::configure(UInt32 serial) noexcept
{
    xdg_surface_send_configure(resource(), serial);
}
