#include <private/LDNDIconRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>

#include <LSurface.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LDNDIconRole::rolePosC() const
{
    m_rolePosC = surface()->posC() - hotspotC();
    return m_rolePosC;
}
//! [rolePosC]

//! [hotspotChanged]
void LDNDIconRole::hotspotChanged() const
{
    /* No default implementation */
}
//! [hotspotChanged]
