#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ;

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

