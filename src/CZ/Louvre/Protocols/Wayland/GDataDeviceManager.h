#ifndef GDATADEVICEMANAGER_H
#define GDATADEVICEMANAGER_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::Wayland::GDataDeviceManager final : public LResource
{
public:
    static void create_data_source(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat) noexcept;
private:
    LGLOBAL_INTERFACE
    GDataDeviceManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GDataDeviceManager() noexcept;
};
#endif // GDATADEVICEMANAGER_H
