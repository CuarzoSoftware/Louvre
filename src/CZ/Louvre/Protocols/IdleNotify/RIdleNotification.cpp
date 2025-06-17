#include <CZ/Louvre/Protocols/IdleNotify/ext-idle-notify-v1.h>
#include <CZ/Louvre/Protocols/IdleNotify/RIdleNotification.h>
#include <CZ/Louvre/LSeat.h>

using namespace Louvre::Protocols::IdleNotify;

static const struct ext_idle_notification_v1_interface imp
{
    .destroy = &RIdleNotification::destroy,
};

RIdleNotification::RIdleNotification
    (
        LClient *client,
        Int32 version,
        UInt32 id,
        UInt32 timeout
    ) noexcept
    :LResource
    (
        client,
        &ext_idle_notification_v1_interface,
        version,
        id,
        &imp
    ),
    m_timeout(timeout == 0 ? 1 : timeout),
    m_timer([this](LTimer *timer)
    {
        seat()->onIdleListenerTimeout(listener());

        if (!timer->running())
            idled();
    })
{
    m_listener.resetTimer();
}

RIdleNotification::~RIdleNotification() noexcept
{
    m_timer.cancel();
}

/******************** REQUESTS ********************/

void RIdleNotification::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RIdleNotification::idled() noexcept
{
    if (!m_idle)
    {
        ext_idle_notification_v1_send_idled(resource());
        m_idle = true;
    }
}

void RIdleNotification::resumed() noexcept
{
    if (m_idle)
    {
        ext_idle_notification_v1_send_resumed(resource());
        m_idle = false;
    }
}
