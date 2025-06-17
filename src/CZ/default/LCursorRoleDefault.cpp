#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/LSurface.h>

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
