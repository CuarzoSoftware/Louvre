#include <private/LFactory.h>
#include <LFactoryObject.h>
#include <cassert>

using namespace Louvre;

LFactoryObject::LFactoryObject(Type type) noexcept : m_type(type)
{
    assert((Int32)type == LFactory::objectBeingCreatedType() && "LFactoryObject instances can only be created from LCompositor::createObjectRequest().");
}
