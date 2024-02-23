#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LTouchPoint.h>
#include <LTouchDownEvent.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LToplevelRole::rolePos() const
{
    m_rolePos = surface()->pos() - xdgSurfaceResource()->imp()->currentWindowGeometry.topLeft();
    return m_rolePos;
}
//! [rolePos]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest(const LEvent &triggeringEvent)
{
    if (fullscreen())
        return;

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        if (!cursor()->output())
            return;

        auto touchDownEvent = (const LTouchDownEvent&)triggeringEvent;
        auto touchPoint = seat()->touch()->findTouchPoint(touchDownEvent.id());

        if (!touchPoint)
            return;

        const LPoint initDragPoint = LTouch::toGlobal(cursor()->output(), touchPoint->pos());

        startMoveSession(triggeringEvent, initDragPoint);
    }
    else
        startMoveSession(triggeringEvent, cursor()->pos());
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(ResizeEdge edge)
{
    /* TODO
    if (!fullscreen() && seat()->pointer()->focus() == surface())
        seat()->pointer()->startResizingToplevel(this, edge, cursor()->pos()); */
}
//! [startResizeRequest]

//! [resizingChanged]
void LToplevelRole::resizingChanged()
{
    /* No default implementation */
}
//! [resizingChanged]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    // Request the client to draw its own window decorations
    setDecorationMode(ClientSide);

    // Activates the toplevel with size (0,0) so that the client can decide the size
    configure(LSize(0,0), pendingStates() | Activated);
}
//! [configureRequest]

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
    if (!cursor()->output())
        return;

    configure(cursor()->output()->size(), Activated | Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    configure(pendingStates() &~ Maximized);
}
//! [unsetMaximizedRequest]

//! [maximizedChanged]
void LToplevelRole::maximizedChanged()
{
    if (maximized())
    {
        if (cursor()->output())
        {
            surface()->raise();
            surface()->setPos(cursor()->output()->pos());
            surface()->setMinimized(false);
        }
        else
            configure(LSize(0, 0), pendingStates() & ~Maximized);
    }
}
//! [maximizedChanged]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *preferredOutput)
{
    const LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output)
        return;

    configure(output->size(), Activated | Fullscreen);
}
//! [setFullscreenRequest]

//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    configure(pendingStates() &~ Fullscreen);
}
//! [unsetFullscreenRequest]

//! [fullscreenChanged]
void LToplevelRole::fullscreenChanged()
{
    if (fullscreen())
    {
        if (cursor()->output())
        {
            surface()->setPos(cursor()->output()->pos());
            surface()->raise();
        }
        else
            configure(LSize(0, 0), pendingStates() & ~Fullscreen);
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

//! [stateChanged]
void LToplevelRole::statesChanged()
{
    /* No default implementation */
}
//! [stateChanged]

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

    if (surface() == seat()->pointer()->focus())
        seat()->pointer()->setFocus(nullptr);

    if (surface() == seat()->keyboard()->focus())
        seat()->keyboard()->setFocus(nullptr);

    if (moveSession())
        moveSession()->stop();

    if (resizeSession())
        resizeSession()->stop();
}
//! [setMinimizedRequest]

//! [showWindowMenuRequest]
void LToplevelRole::showWindowMenuRequest(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequest]
