#ifndef GSESSIONLOCKMANAGER_H
#define GSESSIONLOCKMANAGER_H

#include <LResource.h>
#include <protocols/SessionLock/ext-session-lock-v1.h>

class Louvre::Protocols::SessionLock::GSessionLockManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void lock(wl_client *client, wl_resource *resource, UInt32 id) noexcept;

private:
    GSessionLockManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GSessionLockManager() noexcept;
};


#endif // GSESSIONLOCKMANAGER_H
