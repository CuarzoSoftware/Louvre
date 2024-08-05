#ifndef LIDLELISTENER_H
#define LIDLELISTENER_H

#include <LObject.h>

// TODO: add doc
class Louvre::LIdleListener final : public LObject
{
public:
    void resetTimer() const noexcept;
    LClient *client() const noexcept;
    UInt32 timeout() const noexcept;

    const Protocols::IdleNotify::RIdleNotification &resource() const noexcept
    {
        return m_resource;
    }

private:
    friend class Protocols::IdleNotify::RIdleNotification;
    LIdleListener(Protocols::IdleNotify::RIdleNotification &resource) noexcept;
    ~LIdleListener() noexcept;
    Protocols::IdleNotify::RIdleNotification &m_resource;
};

#endif // LIDLELISTENER_H
