#include <LSubsurfaceRole.h>
#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LSubsurfaceRole::rolePosC() const
{
    if(surface()->parent())
        m_rolePosC = localPosC() + surface()->parent()->rolePosC();

    return m_rolePosC;
}
//! [rolePosC]

//! [localPosChanged]
void LSubsurfaceRole::localPosChanged()
{
    surface()->repaintOutputs();
}
//! [localPosChanged]

//! [syncModeChanged]
void LSubsurfaceRole::syncModeChanged()
{
    /* No default implementation */
}
//! [syncModeChanged]

//! [placedAbove]
void LSubsurfaceRole::placedAbove(LSurface *sibiling)
{
    L_UNUSED(sibiling);
}
//! [placedAbove]

//! [placedBelow]
void LSubsurfaceRole::placedBelow(LSurface *sibiling)
{
    L_UNUSED(sibiling);
}
//! [placedBelow]
