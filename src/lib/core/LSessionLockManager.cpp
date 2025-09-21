#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <private/LOutputPrivate.h>
#include <private/LSurfacePrivate.h>
#include <protocols/SessionLock/RSessionLock.h>

#include <cassert>

using namespace Louvre;

LSessionLockManager::LSessionLockManager(const void *params) noexcept
    : LFactoryObject(FactoryObjectType) {
  assert(params != nullptr &&
         "Invalid parameter passed to LSessionLockManager constructor.");
  LSessionLockManager **ptr{(LSessionLockManager **)params};
  assert(*ptr == nullptr && *ptr == compositor()->sessionLockManager() &&
         "Only a single LSessionLockManager instance can exist.");
  *ptr = this;

  m_sessionLockRes.setOnDestroyCallback([this](auto) {
    if (m_state == Locked) stateChanged();
  });
}

LClient *LSessionLockManager::client() const noexcept {
  return m_sessionLockRes == nullptr ? nullptr : m_sessionLockRes->client();
}

const std::vector<LSessionLockRole *> &LSessionLockManager::roles()
    const noexcept {
  return m_sessionLockRes == nullptr ? m_dummy : m_sessionLockRes->roles();
}

void LSessionLockManager::forceUnlock() {
  if (state() == Unlocked) return;

  if (m_sessionLockRes) {
    for (LSessionLockRole *role : roles())
      if (role->surface()) role->surface()->imp()->setMapped(false);

    for (LOutput *output : seat()->outputs())
      output->imp()->sessionLockRole.reset();

    m_sessionLockRes->finished();
    m_sessionLockRes.reset();
  }

  m_state = Unlocked;
  stateChanged();
}
