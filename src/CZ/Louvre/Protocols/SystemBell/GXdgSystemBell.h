#ifndef GXDGSYSTEMBELL_H
#define GXDGSYSTEMBELL_H

#include <CZ/Louvre/LResource.h>

/// xdg_system_bell_v1: clients ring the system bell (see LSeat::systemBellRequest())
class CZ::Protocols::SystemBell::GXdgSystemBell final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void ring(wl_client *client, wl_resource *resource, wl_resource *surface) noexcept;
private:
    LGLOBAL_INTERFACE
    GXdgSystemBell(wl_client *client, Int32 version, UInt32 id);
};

#endif // GXDGSYSTEMBELL_H
