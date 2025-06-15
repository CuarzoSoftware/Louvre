#include <CZ/Louvre/Protocols/XdgOutput/xdg-output-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgOutput/GXdgOutputManager.h>
#include <CZ/Louvre/Protocols/XdgOutput/RXdgOutput.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::XdgOutput;

static const struct zxdg_output_manager_v1_interface imp
{
    .destroy = &GXdgOutputManager::destroy,
    .get_xdg_output = &GXdgOutputManager::get_xdg_output
};

void GXdgOutputManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GXdgOutputManager(client, version, id);
}

Int32 GXdgOutputManager::maxVersion() noexcept
{
    return LOUVRE_XDG_OUTPUT_MANAGER_VERSION;
}

const wl_interface *GXdgOutputManager::interface() noexcept
{
    return &zxdg_output_manager_v1_interface;
}

GXdgOutputManager::GXdgOutputManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xdgOutputManagerGlobals.emplace_back(this);
}

GXdgOutputManager::~GXdgOutputManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->xdgOutputManagerGlobals, this);
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
