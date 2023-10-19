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

//! [rolePos]
const LPoint &LToplevelRole::rolePos() const
{
    m_rolePos = surface()->pos() - xdgSurfaceResource()->imp()->currentWindowGeometry.topLeft();
    return m_rolePos;
}
//! [rolePos]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest()
{
    if (!fullscreen() && seat()->pointer()->focusSurface() == surface())
        seat()->pointer()->startMovingToplevel(this, cursor()->pos());
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(ResizeEdge edge)
{
    if (!fullscreen() && seat()->pointer()->focusSurface() == surface())
        seat()->pointer()->startResizingToplevel(this, edge, cursor()->pos());
}
//! [startResizeRequest]

//! [resizingChanged]
void LToplevelRole::resizingChanged()
{

}
//! [resizingChanged]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    // Request the client to draw its own window decorations
    setDecorationMode(ClientSide);

    // Activates the toplevel with size (0,0) so that the client can decide the size
    configure(LSize(0,0), states() | Activated);
}
//! [configureRequest]

//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    configure(states() &~ Fullscreen);
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
    if (resizing())
        updateResizingPos();
}
//! [geometryChanged]

//! [decorationModeChanged]
void LToplevelRole::decorationModeChanged()
{
    /* No default implementation */
}
//! [decorationModeChanged]

//! [preferredDecorationModeChanged]
void LToplevelRole::preferredDecorationModeChanged()
{
    /* No default implementation */
}
//! [preferredDecorationModeChanged]

//! [setMaximizedRequest]
void LToplevelRole::setMaximizedRequest()
{
    LOutput *output = compositor()->cursor()->output();
    configure(output->size(), Activated | Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    configure(states() &~ Maximized);
}
//! [unsetMaximizedRequest]

//! [maximizedChanged]
void LToplevelRole::maximizedChanged()
{
    LOutput *output = cursor()->output();

    if (maximized())
    {
        surface()->raise();
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
        surface()->raise();
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

//! [statesChanged]
void LToplevelRole::statesChanged()
{
    /* No default implementation */
}
//! [statesChanged]

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

    configure(output->size(), Activated | Fullscreen);
}
//! [setFullscreenRequest]

//! [showWindowMenuRequest]
void LToplevelRole::showWindowMenuRequest(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequest]
