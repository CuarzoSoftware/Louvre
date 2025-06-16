#include <LCursorRole.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LCursorRole::rolePos() const
{
   return surface()->pos() - hotspot();
}
//! [rolePos]

//! [hotspotChanged]
void LCursorRole::hotspotChanged()
{
    /* No default implementation */
}
//! [hotspotChanged]
