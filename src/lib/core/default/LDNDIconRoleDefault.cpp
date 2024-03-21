#include <private/LDNDIconRolePrivate.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LDNDIconRole::rolePos() const
{
    m_rolePos = surface()->pos() - hotspot();
    return m_rolePos;
}
//! [rolePos]

//! [hotspotChanged]
void LDNDIconRole::hotspotChanged() const
{
    /* No default implementation */
}
//! [hotspotChanged]
