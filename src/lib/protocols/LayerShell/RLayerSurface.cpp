#include <protocols/LayerShell/wlr-layer-shell-unstable-v1.h>
#include <protocols/LayerShell/GLayerShell.h>
#include <protocols/LayerShell/RLayerSurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LLayerRolePrivate.h>
#include <private/LFactory.h>

using namespace Louvre::Protocols::LayerShell;

static const struct zwlr_layer_surface_v1_interface imp
{
    .set_size = RLayerSurface::set_size,
    .set_anchor = RLayerSurface::set_anchor,
    .set_exclusive_zone = RLayerSurface::set_exclusive_zone,
    .set_margin = RLayerSurface::set_margin,
    .set_keyboard_interactivity = RLayerSurface::set_keyboard_interactivity,
    .get_popup = RLayerSurface::get_popup,
    .ack_configure = RLayerSurface::ack_configure,
    .destroy = RLayerSurface::destroy,
#if LOUVRE_LAYER_SHELL_VERSION >= 2
    .set_layer = RLayerSurface::set_layer,
#endif
#if LOUVRE_LAYER_SHELL_VERSION >= 5
    .set_exclusive_edge = RLayerSurface::set_exclusive_edge
#endif
};

RLayerSurface::RLayerSurface(
    UInt32 id,
    GLayerShell *layerShellRes,
    LSurface *surface,
    LOutput *output,
    LSurfaceLayer layer,
    const char *nameSpace) noexcept :
    LResource(
        layerShellRes->client(),
        &zwlr_layer_surface_v1_interface,
        layerShellRes->version(),
        id,
        &imp)
{
    LLayerRole::Params params
    {
        .layerSurfaceRes = this,
        .surface = surface,
        .output = output,
        .layer = layer,
        .nameSpace = nameSpace,
    };

    m_layerRole.reset(LFactory::createObject<LLayerRole>(&params));
    surface->imp()->setPendingRole(m_layerRole.get());
    surface->imp()->applyPendingRole();
}

RLayerSurface::~RLayerSurface()
{

}

void RLayerSurface::set_size(wl_client */*client*/, wl_resource *resource, UInt32 width, UInt32 height)
{
    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;

    res.layerRole()->pendingProps().size.setW(width);
    res.layerRole()->pendingProps().size.setW(height);
    res.layerRole()->m_flags.add(LLayerRole::HasPendingSize);
}

void RLayerSurface::set_anchor(wl_client */*client*/, wl_resource *resource, UInt32 anchor)
{
    using Edge = LLayerRole::Edge;
    anchor &= Edge::Top | Edge::Bottom | Edge::Left | Edge::Right;
    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().anchor = anchor;
    res.layerRole()->m_flags.add(LLayerRole::HasPendingAnchor);
}

void RLayerSurface::set_exclusive_zone(wl_client */*client*/, wl_resource *resource, Int32 zone)
{
    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().exclusiveZone = zone;
    res.layerRole()->m_flags.add(LLayerRole::HasPendingExclusiveZone);
}

void RLayerSurface::set_margin(wl_client */*client*/, wl_resource *resource, Int32 top, Int32 right, Int32 bottom, Int32 left)
{
    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().margin.top = top;
    res.layerRole()->pendingProps().margin.right = right;
    res.layerRole()->pendingProps().margin.bottom = bottom;
    res.layerRole()->pendingProps().margin.left = left;
    res.layerRole()->m_flags.add(LLayerRole::HasPendingMargin);
}

void RLayerSurface::set_keyboard_interactivity(wl_client */*client*/, wl_resource *resource, UInt32 keyboard_interactivity)
{
    if (keyboard_interactivity > 2)
    {
        wl_resource_post_error(resource, ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_KEYBOARD_INTERACTIVITY, "Invalid keyboard interactivity value.");
        return;
    }

    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().keyboardInteractivity = (LLayerRole::KeyboardInteractivity)keyboard_interactivity;
    res.layerRole()->m_flags.add(LLayerRole::HasPendingKeyboardInteractivity);
}

void RLayerSurface::get_popup(wl_client */*client*/, wl_resource *resource, wl_resource *popup)
{

}

void RLayerSurface::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{

}

void RLayerSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

#if LOUVRE_LAYER_SHELL_VERSION >= 2
void RLayerSurface::set_layer(wl_client */*client*/, wl_resource *resource, UInt32 layer)
{
    if (layer > 3)
    {
        wl_resource_post_error(resource, ZWLR_LAYER_SHELL_V1_ERROR_INVALID_LAYER, "Invalid layer value.");
        return;
    }

    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().layer = static_cast<LSurfaceLayer>(layer < 2 ? layer : layer + 1);
    res.layerRole()->m_flags.add(LLayerRole::HasPendingLayer);
}
#endif

#if LOUVRE_LAYER_SHELL_VERSION >= 5
void RLayerSurface::set_exclusive_edge(wl_client */*client*/, wl_resource *resource, UInt32 edge)
{
    if (!(edge == 0 || edge == 1 || edge == 2 || edge == 4 || edge == 8))
    {
        wl_resource_post_error(resource, ZWLR_LAYER_SURFACE_V1_ERROR_INVALID_EXCLUSIVE_EDGE, "Invalid exclusive edge.");
        return;
    }

    auto &res { *static_cast<RLayerSurface*>(wl_resource_get_user_data(resource)) };
    res.layerRole()->pendingProps().exclusiveEdge = (LLayerRole::Edge)edge;
    res.layerRole()->m_flags.add(LLayerRole::HasPendingExclusiveEdge);
}
#endif

void RLayerSurface::configure(UInt32 serial, const LSize &size) noexcept
{
    zwlr_layer_surface_v1_send_configure(resource(), serial, size.w(), size.h());
}

void RLayerSurface::closed() noexcept
{
    zwlr_layer_surface_v1_send_closed(resource());
}
