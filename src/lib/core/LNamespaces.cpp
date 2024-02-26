#include <LNamespaces.h>
#include <LObject.h>

using namespace Louvre;

LWeakData *PrivateUtils::getObjectData(const LObject *object)
{
    return object->m_weakData;
}
