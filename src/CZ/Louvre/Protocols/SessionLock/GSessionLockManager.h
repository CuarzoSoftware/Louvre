#ifndef GSESSIONLOCKMANAGER_H
#define GSESSIONLOCKMANAGER_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::SessionLock::GSessionLockManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void lock(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
private:
    LGLOBAL_INTERFACE
    GSessionLockManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GSessionLockManager() noexcept;
};

#endif // GSESSIONLOCKMANAGER_H
