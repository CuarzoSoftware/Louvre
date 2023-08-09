#include <LOutput.h>
#include <LSurface.h>
#include <LToplevelRole.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LCursor.h>

using namespace Louvre;

//! [damaged]
void LSurface::damaged()
{
    repaintOutputs();
}
//! [damaged]

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
    // If the surface is a Toplevel, we place it in the center of the screen
    if (mapped() && toplevel())
        setPos(cursor()->output()->pos() + (cursor()->output()->size() - size()) / 2);

    compositor()->repaintAllOutputs();
}
//! [mappingChanged]

//! [bufferScaleChanged]
void LSurface::bufferScaleChanged()
{
    repaintOutputs();
}
//! [bufferScaleChanged]

//! [bufferSizeChanged]
void LSurface::bufferSizeChanged()
{
    repaintOutputs();
}
//! [bufferSizeChanged]

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

//! [raised]
void LSurface::raised()
{
    repaintOutputs();
}
//! [raised]

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
