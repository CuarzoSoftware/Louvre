#ifndef GCOMPOSITOR_H
#define GCOMPOSITOR_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::Wayland::GCompositor final : public LResource
{
public:
    static void create_surface(wl_client *client, wl_resource *resource, UInt32 id);
    static void create_region (wl_client *client, wl_resource *resource, UInt32 id) noexcept;
private:
    LGLOBAL_INTERFACE
    GCompositor(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GCompositor() noexcept;
};

#endif // GCOMPOSITOR_H
