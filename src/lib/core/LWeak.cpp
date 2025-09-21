#include <LObject.h>
#include <LWeak.h>

using namespace Louvre;

std::vector<void *> &LWeakUtils::objectRefs(const LObject *object) noexcept {
  return object->m_weakRefs;
}

bool LWeakUtils::isObjectDestroyed(const LObject *object) noexcept {
  return object->m_destroyed;
}
