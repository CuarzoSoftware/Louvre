#ifndef RSESSIONLOCK_H
#define RSESSIONLOCK_H

#include <LResource.h>

class Louvre::Protocols::SessionLock::RSessionLock final : public LResource
{
public:

    enum Reply : UInt8
    {
        Undefined,
        Locked,
        Finished
    };

    const std::vector<LSessionLockRole*> &roles() const noexcept
    {
        return m_roles;
    }

    Reply reply() const noexcept
    {
        return m_reply;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void get_lock_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output);
    static void unlock_and_destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    // Since 1
    void locked() noexcept;
    void finished() noexcept;

private:
    friend class Louvre::Protocols::SessionLock::GSessionLockManager;
    friend class Louvre::Protocols::SessionLock::RSessionLockSurface;
    RSessionLock(GSessionLockManager *sessionLockManagerRes, UInt32 id) noexcept;
    ~RSessionLock() noexcept = default;
    std::vector<LSessionLockRole*> m_roles;
    Reply m_reply { Undefined };
    bool m_lockedOnce { false };
};

#endif // RSESSIONLOCK_H
