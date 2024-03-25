#ifndef RSESSIONLOCKSURFACE_H
#define RSESSIONLOCKSURFACE_H

#include <LResource.h>
#include <LWeak.h>
#include <memory>

class Louvre::Protocols::SessionLock::RSessionLockSurface final : public LResource
{
public:

    LSessionLockRole *sessionLockRole() const noexcept
    {
        return m_sessionLockRole.get();
    }

    RSessionLock *sessionLockRes() const noexcept
    {
        return m_sessionLockRes.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);

    /******************** EVENTS ********************/

    // Since 1
    void configure(UInt32 serial, UInt32 width, UInt32 height) noexcept;

private:
    friend class Louvre::Protocols::SessionLock::RSessionLock;
    RSessionLockSurface(RSessionLock *sessionLockRes, LSurface *surface, LOutput *output, UInt32 id) ;
    ~RSessionLockSurface();
    std::unique_ptr<LSessionLockRole> m_sessionLockRole;
    LWeak<RSessionLock> m_sessionLockRes;
};

#endif // RSESSIONLOCKSURFACE_H
