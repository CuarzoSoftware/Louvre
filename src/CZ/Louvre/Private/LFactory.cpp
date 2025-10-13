#include <CZ/Louvre/Private/LFactory.h>

using namespace CZ;

static Int32 objectBeingCreated { -1 };

Int32 LFactory::objectBeingCreatedType() noexcept
{
    return objectBeingCreated;
}

void LFactory::setObjectBeingCreatedType(Int32 type) noexcept
{
    objectBeingCreated = type;
}
