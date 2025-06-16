#include <LSessionLockRole.h>
#include <LOutput.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LSessionLockRole::rolePos() const
{
    if (exclusiveOutput())
        return exclusiveOutput()->pos();

    return surface()->pos();
}
//! [rolePos]
