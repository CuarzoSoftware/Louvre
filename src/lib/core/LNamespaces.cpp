#include <LNamespaces.h>
#include <LObject.h>

using namespace Louvre;

std::vector<void *> &PrivateUtils::getObjectData(const LObject *object) noexcept
{
    return object->m_weakRefs;
}

bool PrivateUtils::isObjectDestroyed(const LObject *object) noexcept
{
    return object->m_destroyed;
}
