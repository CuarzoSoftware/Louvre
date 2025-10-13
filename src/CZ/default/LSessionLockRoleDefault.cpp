#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ;

//! [rolePos]
SkIPoint LSessionLockRole::rolePos() const
{
    if (exclusiveOutput())
        return exclusiveOutput()->pos();

    return surface()->pos();
}
//! [rolePos]
