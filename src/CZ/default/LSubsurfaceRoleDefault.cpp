#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LCompositor.h>

using namespace CZ;

//! [rolePos]
SkIPoint LSubsurfaceRole::rolePos() const
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
    CZ_UNUSED(sibling);
}
//! [placedAbove]

//! [placedBelow]
void LSubsurfaceRole::placedBelow(LSurface *sibling)
{
    CZ_UNUSED(sibling);
}
//! [placedBelow]
