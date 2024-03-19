#ifndef GDATADEVICEMANAGER_H
#define GDATADEVICEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GDataDeviceManager final : public LResource
{
public:
    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void create_data_source(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat) noexcept;

private:
    ~GDataDeviceManager() noexcept;
    GDataDeviceManager(wl_client *client,
                       Int32 version,
                       UInt32 id) noexcept;
};
#endif // GDATADEVICEMANAGER_H
