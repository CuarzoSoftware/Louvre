#ifndef GTEARINGCONTROLMANAGER_H
#define GTEARINGCONTROLMANAGER_H

#include <LResource.h>
#include <protocols/TearingControl/tearing-control-v1.h>

class Louvre::Protocols::TearingControl::GTearingControlManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_tearing_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;

private:
    GTearingControlManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GTearingControlManager() noexcept;
};

#endif // GTEARINGCONTROLMANAGER_H
