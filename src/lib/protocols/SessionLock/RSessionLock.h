#ifndef RSESSIONLOCK_H
#define RSESSIONLOCK_H

#include <LResource.h>
#include <LTimer.h>
#include <LWeak.h>

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

    bool makeLockRequest();

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void get_lock_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output);
    static void unlock_and_destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    // Since 1
    void locked() noexcept;
    void finished() noexcept;

private:
    friend class Louvre::LOutput;
    friend class Louvre::LSessionLockRole;
    friend class Louvre::Protocols::SessionLock::GSessionLockManager;
    friend class Louvre::Protocols::SessionLock::RSessionLockSurface;
    RSessionLock(GSessionLockManager *sessionLockManagerRes, UInt32 id) noexcept;
    ~RSessionLock() noexcept { m_timer.cancel(); }
    LTimer m_timer;
    std::vector<LSessionLockRole*> m_roles;
    std::vector<LOutput*> m_pendingRepaint;
    Reply m_reply { Undefined };
    bool m_lockedOnce { false };
};

#endif // RSESSIONLOCK_H
