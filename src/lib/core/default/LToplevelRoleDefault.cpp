#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
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
    m_rolePos = surface()->pos() - windowGeometry().topLeft() + LPoint(extraGeometry().left, extraGeometry().top);
    return m_rolePos;
}
//! [rolePos]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest(const LEvent &triggeringEvent)
{
    if (fullscreen())
        return;

    LOutput *activeOutput { cursor()->output() };

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        if (!activeOutput)
            return;

        const LTouchDownEvent& touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
            return;

        if (touchPoint->surface() != surface())
            return;

        const LPoint initDragPoint { LTouch::toGlobal(activeOutput, touchPoint->pos()) };

        moveSession().start(triggeringEvent, initDragPoint);
    }
    else if (surface()->hasPointerFocus())
    {
        moveSession().start(triggeringEvent, cursor()->pos());
    }
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(const LEvent &triggeringEvent, LBitset<LEdge> edge)
{
    if (fullscreen())
        return;

    LOutput *activeOutput { cursor()->output() };

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        if (!activeOutput)
            return;

        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
            return;

        if (touchPoint->surface() != surface())
            return;

        const LPoint initDragPoint { LTouch::toGlobal(activeOutput, touchPoint->pos()) };

        resizeSession().start(triggeringEvent, edge, initDragPoint);
    }
    else if (surface()->hasPointerFocus())
        resizeSession().start(triggeringEvent, edge, cursor()->pos());
}
//! [startResizeRequest]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    // Sending (0,0) allows the client to decide the size
    configureSize(0,0);
    configureState(pendingConfiguration().state | Activated);
    configureDecorationMode(ClientSide);
    configureCapabilities(WindowMenuCap | FullscreenCap | MaximizeCap | FullscreenCap);
    if (cursor()->output())
        configureBounds(cursor()->output()->availableGeometry().size());
    else
        configureBounds(0, 0);
}
//! [configureRequest]
//!
void LToplevelRole::atomsChanged(LBitset<AtomChanges> changes, const Atoms &prevAtoms)
{
    surface()->repaintOutputs();

    if (!changes.check(StateChanged))
        return;

    const LBitset<State> stateChanges { state() ^ prevAtoms.state };

    if (stateChanges.check(Maximized))
    {
        if (maximized())
        {
            if (exclusiveOutput())
            {
                surface()->raise();
                surface()->setPos(exclusiveOutput()->pos() + exclusiveOutput()->availableGeometry().pos());
                surface()->setMinimized(false);
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().state & ~Maximized);
            }
        }
        else
        {
            surface()->setPos(prevRect.pos());
        }
    }

    if (stateChanges.check(Fullscreen))
    {
        if (fullscreen())
        {
            if (exclusiveOutput())
            {
                surface()->setPos(exclusiveOutput()->pos());
                surface()->raise();
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().state & ~Fullscreen);
            }
        }
        else
        {
            surface()->setPos(prevRect.pos());
        }
    }

    if (stateChanges.check(Fullscreen | Maximized) && !pendingConfiguration().state.check(Fullscreen | Maximized))
        setExclusiveOutput(nullptr);
}

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

//! [preferredDecorationModeChanged]
void LToplevelRole::preferredDecorationModeChanged()
{
    /* No default implementation */
}
//! [preferredDecorationModeChanged]

//! [setMaximizedRequest]
void LToplevelRole::setMaximizedRequest()
{
    if (!cursor()->output() || maximized())
        return;

    if (!fullscreen())
        prevRect = LRect(surface()->pos(), windowGeometry().size());

    setExclusiveOutput(cursor()->output());
    configureSize(cursor()->output()->availableGeometry().size()
        - LSize(extraGeometry().left + extraGeometry().right, extraGeometry().top + extraGeometry().bottom));
    configureState(Activated | Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    if (!maximized())
        return;

    configureState(pendingConfiguration().state &~ Maximized);
    configureSize(prevRect.size());
}
//! [unsetMaximizedRequest]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *preferredOutput)
{
    LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output || fullscreen())
        return;

    if (!maximized())
        prevRect = LRect(surface()->pos(), windowGeometry().size());

    setExclusiveOutput(output);
    configureSize(output->size());
    configureState(Activated | Fullscreen);
}
//! [setFullscreenRequest]

//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    if (!fullscreen())
        return;

    configureState(pendingConfiguration().state &~ Fullscreen);
    configureSize(prevRect.size());
}
//! [unsetFullscreenRequest]

//! [setMinimizedRequest]
void LToplevelRole::setMinimizedRequest()
{
    surface()->setMinimized(true);

    if (surface()->hasPointerFocus())
        seat()->pointer()->setFocus(nullptr);

    if (surface()->hasKeyboardFocus())
        seat()->keyboard()->setFocus(nullptr);

    moveSession().stop();
    resizeSession().stop();
}
//! [setMinimizedRequest]

//! [showWindowMenuRequest]
void LToplevelRole::showWindowMenuRequest(const LEvent &triggeringEvent, Int32 x, Int32 y)
{
    L_UNUSED(triggeringEvent);
    L_UNUSED(x);
    L_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequest]
