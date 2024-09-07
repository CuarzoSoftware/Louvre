#ifndef RXWAYLANDROLE_H
#define RXWAYLANDROLE_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::XWaylandShell::RXWaylandSurface final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void set_serial(wl_client *client, wl_resource *resource, UInt32 serial_lo, UInt32 serial_hi);
    static void destroy(wl_client *client, wl_resource *resource);

private:
    friend class Louvre::Protocols::XWaylandShell::GXWaylandShell;
    RXWaylandSurface(Int32 version, LSurface *surface, UInt32 id) noexcept;
    ~RXWaylandSurface() = default;
    LWeak<LSurface> m_surface;
};

#endif // RXWAYLANDROLE_H
