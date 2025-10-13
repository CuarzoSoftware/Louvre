#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/LFactoryObject.h>
#include <cassert>

using namespace CZ;

LFactoryObject::LFactoryObject(Type type) noexcept : m_type(type)
{
    assert((Int32)type == LFactory::objectBeingCreatedType() && "LFactoryObject instances can only be created from LCompositor::createObjectRequest().");
}
