#include <private/LBaseSurfaceRolePrivate.h>

#include <LCursorRole.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LCursorRole::rolePos() const
{
   m_rolePos = surface()->pos() - hotspot();
   return m_rolePos;
}
//! [rolePosC]

//! [hotspotChanged]
void LCursorRole::hotspotChanged()
{
    /* No default implementation */
}
//! [hotspotChanged]
