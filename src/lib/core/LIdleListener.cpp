#include <LIdleListener.h>
#include <LUtils.h>
#include <private/LSeatPrivate.h>
#include <protocols/IdleNotify/RIdleNotification.h>

using namespace Louvre;
using namespace Louvre::Protocols::IdleNotify;

void LIdleListener::resetTimer() const noexcept {
  m_resource.resumed();
  m_resource.m_timer.start(timeout());
}

LClient *LIdleListener::client() const noexcept { return m_resource.client(); }

UInt32 LIdleListener::timeout() const noexcept { return m_resource.m_timeout; }

LIdleListener::LIdleListener(RIdleNotification &resource) noexcept
    : m_resource(resource) {
  seat()->imp()->idleListeners.emplace_back(this);
}

LIdleListener::~LIdleListener() noexcept {
  notifyDestruction();
  LVectorRemoveOneUnordered(seat()->imp()->idleListeners,
                            (const LIdleListener *)this);
}
