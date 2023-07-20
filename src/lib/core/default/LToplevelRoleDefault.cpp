#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LToplevelRole::rolePos() const
{
    m_rolePos = surface()->pos() - xdgSurfaceResource()->imp()->currentWindowGeometry.topLeft();
    return m_rolePos;
}
//! [rolePosC]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest()
{
    if (!fullscreen() && seat()->pointer()->focusSurface() == surface())
        seat()->pointer()->startMovingToplevel(this);
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(ResizeEdge edge)
{
    if (!fullscreen() && seat()->pointer()->focusSurface() == surface())
        seat()->pointer()->startResizingToplevel(this, edge);
}
//! [startResizeRequest]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    // Request the client to draw its own window decorations
    setDecorationMode(ClientSide);

    // Activates the Toplevel with size (0,0) so that the client can decide the size
    configure(LSize(0,0), states() | LToplevelRole::Activated);
}
//! [configureRequest]


//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    configure(states() &~ LToplevelRole::Fullscreen);
}
//! [unsetFullscreenRequest]

//! [titleChanged]
void LToplevelRole::titleChanged()
{
    /* No default implementation */
}
//! [titleChanged]


//! [appIdChanged]
void LToplevelRole::appIdChanged()
{
    /* No default implementation */
}
//! [appIdChanged]

//! [geometryChanged]
void LToplevelRole::geometryChanged()
{
    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->updateResizingToplevelPos();
}
//! [geometryChanged]

//! [decorationModeChanged]
void LToplevelRole::decorationModeChanged()
{
    /* No default implementation */
}
//! [decorationModeChanged]


//! [setMaximizedRequest]
void LToplevelRole::setMaximizedRequest()
{
    LOutput *output = compositor()->cursor()->output();
    configure(output->size(), LToplevelRole::Activated);
    configure(output->size(), LToplevelRole::Activated | LToplevelRole::Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    configure(states() &~ LToplevelRole::Maximized);
}
//! [unsetMaximizedRequest]

//! [maximizedChanged]
void LToplevelRole::maximizedChanged()
{
    LOutput *output = cursor()->output();

    if (maximized())
    {
        compositor()->raiseSurface(surface());
        surface()->setPos(output->pos());
        surface()->setMinimized(false);
    }
}
//! [maximizedChanged]

//! [fullscreenChanged]
void LToplevelRole::fullscreenChanged()
{
    if (fullscreen())
    {
        surface()->setPos(cursor()->output()->pos());
        compositor()->raiseSurface(surface());
    }
}
//! [fullscreenChanged]

//! [activatedChanged]
void LToplevelRole::activatedChanged()
{
    if (activated())
        seat()->keyboard()->setFocus(surface());

    surface()->repaintOutputs();
}
//! [activatedChanged]

//! [maxSizeChanged]
void LToplevelRole::maxSizeChanged()
{
    /* No default implementation */
}
//! [maxSizeChanged]

//! [minSizeChanged]
void LToplevelRole::minSizeChanged()
{
    /* No default implementation */
}
//! [minSizeChanged]

//! [setMinimizedRequest]
void LToplevelRole::setMinimizedRequest()
{
    surface()->setMinimized(true);

    if (surface() == seat()->pointer()->focusSurface())
        seat()->pointer()->setFocus(nullptr);

    if (surface() == seat()->keyboard()->focusSurface())
        seat()->keyboard()->setFocus(nullptr);

    if (this == seat()->pointer()->movingToplevel())
        seat()->pointer()->stopMovingToplevel();

    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->stopResizingToplevel();
}
//! [setMinimizedRequest]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *destOutput)
{
    LOutput *output;

    if (destOutput)
        output = destOutput;
    else
        output = cursor()->output();

    configure(output->size(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
}
//! [setFullscreenRequest]

//! [showWindowMenuRequestS]
void LToplevelRole::showWindowMenuRequestS(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequestS]
