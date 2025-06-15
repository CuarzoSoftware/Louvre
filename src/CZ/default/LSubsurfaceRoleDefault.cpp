#include <LSubsurfaceRole.h>
#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LSubsurfaceRole::rolePos() const
{
    if (surface()->parent())
        m_rolePos = localPos() + surface()->parent()->rolePos();

    return m_rolePos;
}
//! [rolePos]

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
void LSubsurfaceRole::placedAbove(LSurface *sibling)
{
    L_UNUSED(sibling);
}
//! [placedAbove]

//! [placedBelow]
void LSubsurfaceRole::placedBelow(LSurface *sibling)
{
    L_UNUSED(sibling);
}
//! [placedBelow]
