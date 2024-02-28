#include <LNamespaces.h>
#include <LObject.h>

using namespace Louvre;

std::vector<void *> &PrivateUtils::getObjectData(const LObject *object)
{
    return object->m_weakRefs;
}
