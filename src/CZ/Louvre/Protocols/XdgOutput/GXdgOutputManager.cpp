#include <CZ/Louvre/Protocols/XdgOutput/xdg-output-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgOutput/GXdgOutputManager.h>
#include <CZ/Louvre/Protocols/XdgOutput/RXdgOutput.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::XdgOutput;

static const struct zxdg_output_manager_v1_interface imp
{
    .destroy = &GXdgOutputManager::destroy,
    .get_xdg_output = &GXdgOutputManager::get_xdg_output
};

LGLOBAL_INTERFACE_IMP(GXdgOutputManager, LOUVRE_XDG_OUTPUT_MANAGER_VERSION, zxdg_output_manager_v1_interface)

bool GXdgOutputManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgOutputManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgOutputManager;
    return true;
}

GXdgOutputManager::GXdgOutputManager
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
    this->client()->imp()->xdgOutputManagerGlobals.emplace_back(this);
}

GXdgOutputManager::~GXdgOutputManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->xdgOutputManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GXdgOutputManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgOutputManager::get_xdg_output(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *output) noexcept
{
    new RXdgOutput(static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)),
                   id,
                   wl_resource_get_version(resource));
}
