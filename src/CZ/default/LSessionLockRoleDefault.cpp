#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LSurface.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LSessionLockRole::rolePos() const
{
    if (exclusiveOutput())
        return exclusiveOutput()->pos();

    return surface()->pos();
}
//! [rolePos]
