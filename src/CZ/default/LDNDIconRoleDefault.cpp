#include <CZ/Louvre/Private/LDNDIconRolePrivate.h>
#include <CZ/Louvre/LSurface.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LDNDIconRole::rolePos() const
{
    return surface()->pos() - hotspot();
}
//! [rolePos]

//! [hotspotChanged]
void LDNDIconRole::hotspotChanged() const
{
    /* No default implementation */
}
//! [hotspotChanged]
