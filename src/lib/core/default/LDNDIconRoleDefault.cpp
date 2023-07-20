#include <private/LDNDIconRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>

#include <LSurface.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LDNDIconRole::rolePos() const
{
    m_rolePos = surface()->pos() - hotspot();
    return m_rolePos;
}
//! [rolePosC]

//! [hotspotChanged]
void LDNDIconRole::hotspotChanged() const
{
    /* No default implementation */
}
//! [hotspotChanged]
