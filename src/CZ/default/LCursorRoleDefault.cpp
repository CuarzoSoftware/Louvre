#include <LCursorRole.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LCursorRole::rolePos() const
{
   m_rolePos = surface()->pos() - hotspot();
   return m_rolePos;
}
//! [rolePos]

//! [hotspotChanged]
void LCursorRole::hotspotChanged()
{
    /* No default implementation */
}
//! [hotspotChanged]
