#include <CZ/Louvre/Protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgDecoration/GXdgDecorationManager.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgToplevel.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::XdgDecoration;

static const struct zxdg_decoration_manager_v1_interface imp
{
    .destroy = &GXdgDecorationManager::destroy,
    .get_toplevel_decoration = &GXdgDecorationManager::get_toplevel_decoration
};

void GXdgDecorationManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GXdgDecorationManager(client, version, id);
}

Int32 GXdgDecorationManager::maxVersion() noexcept
{
    return LOUVRE_XDG_DECORATION_MANAGER_VERSION;
}

const wl_interface *GXdgDecorationManager::interface() noexcept
{
    return &zxdg_decoration_manager_v1_interface;
}

GXdgDecorationManager::GXdgDecorationManager(
    wl_client *client,
    Int32 version,
    UInt32 id) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xdgDecorationManagerGlobals.push_back(this);
}

GXdgDecorationManager::~GXdgDecorationManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->xdgDecorationManagerGlobals, this);
}

void GXdgDecorationManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgDecorationManager::get_toplevel_decoration(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *toplevel) noexcept
{
    auto &xdgToplevelRoleRes { *static_cast<XdgShell::RXdgToplevel*>(wl_resource_get_user_data(toplevel)) };

    if (xdgToplevelRoleRes.toplevelRole()->supportServerSideDecorations())
    {
        xdgToplevelRoleRes.postError(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED,
                               "Multiple XDG Toplevel Decorations for a Toplevel not supported.");
        return;
    }

    if (xdgToplevelRoleRes.xdgSurfaceRes()->surface()->imp()->hasBufferOrPendingBuffer())
    {
        xdgToplevelRoleRes.postError(ZXDG_TOPLEVEL_DECORATION_V1_ERROR_UNCONFIGURED_BUFFER,
                               "Given Toplevel already has a buffer attached.");
        return;
    }

    new RXdgToplevelDecoration(static_cast<GXdgDecorationManager*>(wl_resource_get_user_data(resource)),
                               xdgToplevelRoleRes.toplevelRole(),
                               id);
}
