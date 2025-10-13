#ifndef RSESSIONLOCK_H
#define RSESSIONLOCK_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::SessionLock::RSessionLock final : public LResource
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
    friend class CZ::LOutput;
    friend class CZ::LSessionLockRole;
    friend class CZ::Protocols::SessionLock::GSessionLockManager;
    friend class CZ::Protocols::SessionLock::RSessionLockSurface;
    RSessionLock(GSessionLockManager *sessionLockManagerRes, UInt32 id) noexcept;
    ~RSessionLock() noexcept { m_timer.stop(false); }
    CZTimer m_timer;
    std::vector<LSessionLockRole*> m_roles;
    std::vector<LOutput*> m_pendingRepaint;
    Reply m_reply { Undefined };
    bool m_lockedOnce { false };
};

#endif // RSESSIONLOCK_H
