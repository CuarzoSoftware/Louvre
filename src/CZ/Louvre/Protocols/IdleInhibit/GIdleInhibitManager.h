#ifndef GIDLEINHIBITMANAGER_H
#define GIDLEINHIBITMANAGER_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::IdleInhibit::GIdleInhibitManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_inhibitor(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;
private:
    LGLOBAL_INTERFACE
    GIdleInhibitManager(wl_client *client, Int32 version, UInt32 id);
    ~GIdleInhibitManager() noexcept;
};

#endif // GIDLEINHIBITMANAGER_H
