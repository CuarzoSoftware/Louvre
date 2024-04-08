#include <protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <protocols/LayerShell/GLayerShell.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::LayerShell;

static const struct zwlr_layer_shell_v1_interface imp
{
    .get_layer_surface = &GLayerShell::get_layer_surface,
    .destroy = &GLayerShell::destroy
};

void GLayerShell::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GLayerShell(client, version, id);
}

Louvre::Int32 GLayerShell::maxVersion() noexcept
{
    return LOUVRE_LAYER_SHELL_VERSION;
}

const wl_interface *GLayerShell::interface() noexcept
{
    return &zwlr_layer_shell_v1_interface;
}

GLayerShell::GLayerShell(
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
    this->client()->imp()->layerShellGlobals.emplace_back(this);
}

GLayerShell::~GLayerShell() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->layerShellGlobals, this);
}

/******************** REQUESTS ********************/

void GLayerShell::get_layer_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output, UInt32 layer, const char *name_space) noexcept
{

}

void GLayerShell::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
