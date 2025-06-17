#ifndef GXDGOUTPUTMANAGER_H
#define GXDGOUTPUTMANAGER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::XdgOutput::GXdgOutputManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_xdg_output(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output) noexcept;
private:
    LGLOBAL_INTERFACE
    GXdgOutputManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GXdgOutputManager() noexcept;
};

#endif // GXDGOUTPUTMANAGER_H
