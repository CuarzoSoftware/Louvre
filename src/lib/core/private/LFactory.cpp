#include <LFactory.h>

using namespace Louvre;

static Int32 objectBeingCreated { -1 };

Int32 LFactory::objectBeingCreatedType() noexcept
{
    return objectBeingCreated;
}

void LFactory::setObjectBeingCreatedType(Int32 type) noexcept
{
    objectBeingCreated = type;
}
