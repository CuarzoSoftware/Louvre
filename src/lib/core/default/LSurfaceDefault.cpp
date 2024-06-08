#include <LOutput.h>
#include <LSurface.h>
#include <LToplevelRole.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

//! [layerChanged]
void LSurface::layerChanged()
{
    repaintOutputs();
}
//! [layerChanged]

//! [pointerConstraintModeChanged]
void LSurface::pointerConstraintModeChanged()
{
    /* No default implementation */
}
//! [pointerConstraintModeChanged]

//! [pointerConstraintRegionChanged]
void LSurface::pointerConstraintRegionChanged()
{
    /* No default implementation */
}
//! [pointerConstraintRegionChanged]

//! [lockedPointerPosHintChanged]
void LSurface::lockedPointerPosHintChanged()
{
    /* No default implementation */
}
//! [lockedPointerPosHintChanged]

//! [damageChanged]
void LSurface::damageChanged()
{
    repaintOutputs();
}
//! [damageChanged]

//! [roleChanged]
void LSurface::roleChanged()
{
    repaintOutputs();
}
//! [roleChanged]

//! [parentChanged]
void LSurface::parentChanged()
{
    repaintOutputs();
}
//! [parentChanged]

//! [mappingChanged]
void LSurface::mappingChanged()
{
    compositor()->repaintAllOutputs();

    LOutput *activeOutput { cursor()->output() };

    if (!activeOutput)
        return;

    /* If the surface is a toplevel, we place it at the center of the screen */
    if (mapped() && toplevel())
    {
        const LSize size {
            toplevel()->windowGeometry().size()
            + LSize(toplevel()->extraGeometry().top + toplevel()->extraGeometry().bottom,
                    toplevel()->extraGeometry().left + toplevel()->extraGeometry().right)
        };

        const LSize availGeoPos { activeOutput->pos() + activeOutput->availableGeometry().pos() };

        setPos(availGeoPos + (activeOutput->availableGeometry().size() - size) / 2);

        if (pos().y() < availGeoPos.y())
            setY(availGeoPos.y());
    }
}
//! [mappingChanged]

//! [bufferScaleChanged]
void LSurface::bufferScaleChanged()
{
    repaintOutputs();
}
//! [bufferScaleChanged]

//! [bufferTransformChanged]
void LSurface::bufferTransformChanged()
{
    repaintOutputs();
}
//! [bufferTransformChanged]

//! [bufferSizeChanged]
void LSurface::bufferSizeChanged()
{
    repaintOutputs();
}
//! [bufferSizeChanged]

//! [sizeChanged]
void LSurface::sizeChanged()
{
    repaintOutputs();
}
//! [sizeChanged]

//! [srcRectChanged]
void LSurface::srcRectChanged()
{
    repaintOutputs();
}
//! [srcRectChanged]

//! [opaqueRegionChanged]
void LSurface::opaqueRegionChanged()
{
    repaintOutputs();
}
//! [opaqueRegionChanged]

//! [inputRegionChanged]
void LSurface::inputRegionChanged()
{
    /* No default implementation */
}
//! [inputRegionChanged]

//! [orderChanged]
void LSurface::orderChanged()
{
    repaintOutputs();
}
//! [orderChanged]

//! [requestedRepaint]
void LSurface::requestedRepaint()
{
    repaintOutputs();
}
//! [requestedRepaint]

//! [minimizedChanged]
void LSurface::minimizedChanged()
{
    repaintOutputs();
}
//! [minimizedChanged]

//! [preferVSyncChanged]
void LSurface::preferVSyncChanged()
{
    /* No default implementation */
}
//! [preferVSyncChanged]
