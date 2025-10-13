#ifndef GXDGDECORATIONMANAGER_H
#define GXDGDECORATIONMANAGER_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::XdgDecoration::GXdgDecorationManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_toplevel_decoration(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *toplevel) noexcept;
private:
    LGLOBAL_INTERFACE
    GXdgDecorationManager(wl_client *client, Int32 version, UInt32 id);
    ~GXdgDecorationManager() noexcept;
};

#endif // GXDGDECORATIONMANAGER_H
