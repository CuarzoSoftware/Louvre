#ifndef GXWAYLANDSHELL_H
#define GXWAYLANDSHELL_H

#include <LResource.h>

class Louvre::Protocols::XWaylandShell::GXWaylandShell final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_xwayland_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;
private:
    LGLOBAL_INTERFACE
    GXWaylandShell(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GXWaylandShell() noexcept;
};

#endif // GXWAYLANDSHELL_H
