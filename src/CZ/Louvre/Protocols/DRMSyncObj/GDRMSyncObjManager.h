#ifndef GDRMSYNCOBJMANAGER_H
#define GDRMSYNCOBJMANAGER_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::DRMSyncObj::GDRMSyncObjManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;
    static void import_timeline(wl_client *client, wl_resource *resource, UInt32 id, Int32 fd) noexcept;

private:
    LGLOBAL_INTERFACE
    GDRMSyncObjManager(wl_client *client, Int32 version, UInt32 id);
    ~GDRMSyncObjManager() noexcept;
};

#endif // GDRMSYNCOBJMANAGER_H
