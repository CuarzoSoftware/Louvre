#ifndef GLAYERSHELL_H
#define GLAYERSHELL_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::LayerShell::GLayerShell final : public LResource
{
public:
    static void get_layer_surface(wl_client *client, wl_resource *resource,
                                  UInt32 id, wl_resource *surface, wl_resource *output,
                                  UInt32 layer, const char *name_space);

#if LOUVRE_LAYER_SHELL_VERSION >= 3
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
#endif
private:
    LGLOBAL_INTERFACE
    GLayerShell(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GLayerShell() noexcept;
};

#endif // GLAYERSHELL_H
