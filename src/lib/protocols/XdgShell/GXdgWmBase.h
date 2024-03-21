#ifndef GXDGWMBASE_H
#define GXDGWMBASE_H

#include <LResource.h>
#include <protocols/XdgShell/xdg-shell.h>

class Louvre::Protocols::XdgShell::GXdgWmBase final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_positioner(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void get_xdg_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;
    static void pong(wl_client *client, wl_resource *resource, UInt32 serial);

    /******************** EVENTS ********************/

    // Since 1
    void ping(UInt32 serial) noexcept;

private:
    friend class Louvre::Protocols::XdgShell::RXdgSurface;
    GXdgWmBase(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GXdgWmBase() noexcept;
    UInt32 m_xdgSurfacesCount { 0 };
};

#endif // GXDGWMBASE_H
