#ifndef RLAYERSURFACE_H
#define RLAYERSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/Layout/LSurfaceLayer.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

class CZ::Protocols::LayerShell::RLayerSurface final : public LResource
{
public:

    LLayerRole *layerRole() const noexcept
    {
        return m_layerRole.get();
    }

    /******************** REQUESTS ********************/

    static void set_size(wl_client *client, wl_resource *resource, UInt32 width, UInt32 height);
    static void set_anchor(wl_client *client, wl_resource *resource, UInt32 anchor);
    static void set_exclusive_zone(wl_client *client, wl_resource *resource, Int32 zone);
    static void set_margin(wl_client *client, wl_resource *resource, Int32 top, Int32 right, Int32 bottom, Int32 left);
    static void set_keyboard_interactivity(wl_client *client, wl_resource *resource, UInt32 keyboard_interactivity);
    static void get_popup(wl_client *client, wl_resource *resource, wl_resource *popup);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);
    static void destroy(wl_client *client, wl_resource *resource);

#if LOUVRE_LAYER_SHELL_VERSION >= 2
    static void set_layer(wl_client *client, wl_resource *resource, UInt32 layer);
#endif

#if LOUVRE_LAYER_SHELL_VERSION >= 5
    static void set_exclusive_edge(wl_client *client, wl_resource *resource, UInt32 edge);
#endif
    /******************** EVENTS ********************/

    // Since 1
    void configure(UInt32 serial, const SkISize &size) noexcept;
    void closed() noexcept;

private:
    friend class GLayerShell;
    RLayerSurface(UInt32 id, GLayerShell *layerShellRes, LSurface *surface, LOutput *output, LSurfaceLayer layer, const char *scope) noexcept;
    ~RLayerSurface();
    std::unique_ptr<LLayerRole> m_layerRole;
};

#endif // RLAYERSURFACE_H
