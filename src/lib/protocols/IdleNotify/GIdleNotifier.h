#ifndef GIDLENOTIFIER_H
#define GIDLENOTIFIER_H

#include <LResource.h>

class Louvre::Protocols::IdleNotify::GIdleNotifier final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_idle_notification(wl_client *client, wl_resource *resource, UInt32 id, UInt32 timeout, wl_resource *seat) noexcept;
private:
    LGLOBAL_INTERFACE
    GIdleNotifier(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GIdleNotifier() noexcept;
};

#endif // GIDLENOTIFIER_H
