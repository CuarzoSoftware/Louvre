#include <LObject.h>
#include <private/LCompositorPrivate.h>

using namespace Louvre;

LObject::~LObject()
{
    *m_isAlive = false;
    compositor()->imp()->removedObjectsAliveIndicators.emplace_back(std::move(m_isAlive));
}
