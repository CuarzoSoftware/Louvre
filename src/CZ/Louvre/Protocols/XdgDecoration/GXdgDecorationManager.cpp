#include <CZ/Louvre/Protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgDecoration/GXdgDecorationManager.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgToplevel.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::XdgDecoration;

static const struct zxdg_decoration_manager_v1_interface imp
{
    .destroy = &GXdgDecorationManager::destroy,
    .get_toplevel_decoration = &GXdgDecorationManager::get_toplevel_decoration
};

LGLOBAL_INTERFACE_IMP(GXdgDecorationManager, LOUVRE_XDG_DECORATION_MANAGER_VERSION, zxdg_decoration_manager_v1_interface)

bool GXdgDecorationManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgDecorationManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgDecorationManager;
    return true;
}

GXdgDecorationManager::GXdgDecorationManager(
    wl_client *client,
    Int32 version,
    UInt32 id)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xdgDecorationManagerGlobals.push_back(this);
}

GXdgDecorationManager::~GXdgDecorationManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->xdgDecorationManagerGlobals, this);
}

void GXdgDecorationManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgDecorationManager::get_toplevel_decoration(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *toplevel) noexcept
{
    auto &xdgToplevelRoleRes { *static_cast<XdgShell::RXdgToplevel*>(wl_resource_get_user_data(toplevel)) };

    if (xdgToplevelRoleRes.toplevelRole()->supportsSSD())
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
