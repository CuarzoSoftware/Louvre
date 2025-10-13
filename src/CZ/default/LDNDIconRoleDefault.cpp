#include <CZ/Louvre/Private/LDNDIconRolePrivate.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ;

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
