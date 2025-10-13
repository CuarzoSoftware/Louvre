#include <CZ/Louvre/Protocols/IdleNotify/RIdleNotification.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Seat/LIdleListener.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::IdleNotify;

void LIdleListener::resetTimer() const noexcept
{
    m_resource.resumed();
    m_resource.m_timer.start(timeout());
}

LClient *LIdleListener::client() const noexcept
{
    return m_resource.client();
}

UInt32 LIdleListener::timeout() const noexcept
{
    return m_resource.m_timeout;
}

LIdleListener::LIdleListener(RIdleNotification &resource) noexcept : m_resource(resource)
{
    seat()->imp()->idleListeners.emplace_back(this);
}

LIdleListener::~LIdleListener() noexcept
{
    notifyDestruction();
    CZVectorUtils::RemoveOneUnordered(seat()->imp()->idleListeners, (const LIdleListener*)this);
}
