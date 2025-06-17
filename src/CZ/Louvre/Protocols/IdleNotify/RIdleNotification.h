#ifndef RIDLENOTIFICATION_H
#define RIDLENOTIFICATION_H

#include <CZ/Louvre/LIdleListener.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/LTimer.h>

class Louvre::Protocols::IdleNotify::RIdleNotification final : public LResource
{
public:

    const LIdleListener &listener() const noexcept
    {
        return m_listener;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    void idled() noexcept;
    void resumed() noexcept;

private:
    friend class Louvre::Protocols::IdleNotify::GIdleNotifier;
    friend class Louvre::LIdleListener;
    RIdleNotification(LClient *client, Int32 version, UInt32 id, UInt32 timeout) noexcept;
    ~RIdleNotification() noexcept;
    bool m_idle { false };
    UInt32 m_timeout;
    LTimer m_timer;
    LIdleListener m_listener { *this };
};

#endif // RIDLENOTIFICATION_H
