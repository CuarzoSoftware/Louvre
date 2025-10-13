#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPositioner.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::XdgShell;

static const struct xdg_wm_base_interface imp
{
    .destroy = &GXdgWmBase::destroy,
    .create_positioner = &GXdgWmBase::create_positioner,
    .get_xdg_surface = &GXdgWmBase::get_xdg_surface,
    .pong = &GXdgWmBase::pong
};

LGLOBAL_INTERFACE_IMP(GXdgWmBase, LOUVRE_XDG_WM_BASE_VERSION, xdg_wm_base_interface)

bool GXdgWmBase::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgWmBase)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgWmBase;
    return true;
}

XdgShell::GXdgWmBase::GXdgWmBase
(
    wl_client *client,
    Int32 version,
    UInt32 id
)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xdgWmBaseGlobals.push_back(this);
}

GXdgWmBase::~GXdgWmBase() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->xdgWmBaseGlobals, this);
}

/******************** REQUESTS ********************/

void GXdgWmBase::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    auto &res { *static_cast<GXdgWmBase*>(wl_resource_get_user_data(resource)) };

    if (res.m_xdgSurfacesCount != 0)
    {
        res.postError(XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "xdg_wm_base destroyed before all surfaces");
        return;
    }

    wl_resource_destroy(resource);
}

void GXdgWmBase::create_positioner(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RXdgPositioner(static_cast<GXdgWmBase*>(wl_resource_get_user_data(resource)), id);
}

void GXdgWmBase::get_xdg_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto &surfaceRes { *static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };
    auto &res { *static_cast<GXdgWmBase*>(wl_resource_get_user_data(resource)) };

    if (!surfaceRes.surface()->imp()->canHostRole())
    {
        res.postError(XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role or xdg_surface assigned");
        return;
    }

    new RXdgSurface(static_cast<GXdgWmBase*>(wl_resource_get_user_data(resource)), surfaceRes.surface(), id);
}

void GXdgWmBase::pong(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    static_cast<GXdgWmBase*>(wl_resource_get_user_data(resource))->client()->pong(serial);
}

/******************** EVENTS ********************/

void XdgShell::GXdgWmBase::ping(UInt32 serial) noexcept
{
    xdg_wm_base_send_ping(resource(), serial);
}
