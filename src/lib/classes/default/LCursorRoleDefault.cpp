#include <private/LBaseSurfaceRolePrivate.h>

#include <LCursorRole.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LCursorRole::rolePosC() const
{
   m_rolePosC = surface()->posC() - hotspotC();
   return m_rolePosC;
}
//! [rolePosC]

//! [hotspotChanged]
void LCursorRole::hotspotChanged()
{
    /* No default implementation */
}
//! [hotspotChanged]
