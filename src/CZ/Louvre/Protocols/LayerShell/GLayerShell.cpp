#include <CZ/Louvre/Protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <CZ/Louvre/Protocols/LayerShell/GLayerShell.h>
#include <CZ/Louvre/Protocols/LayerShell/RLayerSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::LayerShell;

static const struct zwlr_layer_shell_v1_interface imp
{
    .get_layer_surface = &GLayerShell::get_layer_surface,

#if LOUVRE_LAYER_SHELL_VERSION >= 3
    .destroy = &GLayerShell::destroy,
#else
    .destroy = NULL
#endif
};

void GLayerShell::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GLayerShell(client, version, id);
}

Int32 GLayerShell::maxVersion() noexcept
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

void GLayerShell::get_layer_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output, UInt32 layer, const char *name_space)
{
    auto &surfaceRes { *static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (layer > 3)
    {
        surfaceRes.postError(ZWLR_LAYER_SHELL_V1_ERROR_INVALID_LAYER, "Invalid layer value.");
        return;
    }

    if (surfaceRes.surface()->role())
    {
        surfaceRes.postError(ZWLR_LAYER_SHELL_V1_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    if (surfaceRes.surface()->imp()->hasBufferOrPendingBuffer())
    {
        surfaceRes.postError(ZWLR_LAYER_SHELL_V1_ERROR_ALREADY_CONSTRUCTED, "wl_surface has a buffer attached or committed.");
        return;
    }

    const LSurfaceLayer surfaceLayer { static_cast<LSurfaceLayer>(layer < 2 ? layer : layer + 1) };

    new RLayerSurface(
        id,
        static_cast<GLayerShell*>(wl_resource_get_user_data(resource)),
        surfaceRes.surface(),
        output == NULL ? nullptr : static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output))->output(),
        surfaceLayer,
        name_space);
}

#if LOUVRE_LAYER_SHELL_VERSION >= 3
void GLayerShell::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif
