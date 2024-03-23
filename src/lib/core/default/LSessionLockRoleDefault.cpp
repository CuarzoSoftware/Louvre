#include <LSessionLockRole.h>
#include <LOutput.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LSessionLockRole::rolePos() const
{
    if (output())
        return output()->pos();

    if (surface())
        return surface()->pos();

    return m_rolePos;
}
//! [rolePos]
