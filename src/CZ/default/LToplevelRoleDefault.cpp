#include <LToplevelRole.h>
#include <LTouchPoint.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LSeat.h>
#include <LOutput.h>
#include <LCursor.h>

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
    else if (seat()->pointer()->focus()
             && (surface()->hasPointerFocus()
             || (seat()->pointer()->focus()->subsurface() && seat()->pointer()->focus()->isSubchildOf(surface()))))
    {

        if (triggeringEvent.type() == LEvent::Type::Pointer)
        {
            if (triggeringEvent.subtype() != LEvent::Subtype::Button)
                return;

            const LPointerButtonEvent &pointerButtonEvent { static_cast<const LPointerButtonEvent&>(triggeringEvent) };

            if (!seat()->pointer()->isButtonPressed(pointerButtonEvent.button()))
                return;
        }

        moveSession().start(triggeringEvent, cursor()->pos());
    }
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(const LEvent &triggeringEvent, CZBitset<LEdge> edge)
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
    else if (seat()->pointer()->focus()
             && (surface()->hasPointerFocus()
                 || (seat()->pointer()->focus()->subsurface() && seat()->pointer()->focus()->isSubchildOf(surface()))))
    {
        if (triggeringEvent.type() == LEvent::Type::Pointer)
        {
            if (triggeringEvent.subtype() != LEvent::Subtype::Button)
                return;

            const LPointerButtonEvent &pointerButtonEvent { static_cast<const LPointerButtonEvent&>(triggeringEvent) };

            if (!seat()->pointer()->isButtonPressed(pointerButtonEvent.button()))
                return;
        }

        resizeSession().start(triggeringEvent, edge, cursor()->pos());
    }
}
//! [startResizeRequest]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    LOutput *output { cursor()->output() };

    if (output)
    {
        surface()->sendOutputEnterEvent(output);
        configureBounds(
            output->availableGeometry().size()
            - LSize(extraGeometry().left + extraGeometry().right, extraGeometry().top + extraGeometry().bottom));
    }
    else
        configureBounds(0, 0);

    configureSize(0,0);
    configureState(pendingConfiguration().state | Activated);
    configureDecorationMode(ClientSide);
    configureCapabilities(WindowMenuCap | FullscreenCap | MaximizeCap | MinimizeCap);
}
//! [configureRequest]


//! [atomsChanged]
void LToplevelRole::atomsChanged(CZBitset<AtomChanges> changes, const Atoms &prevAtoms)
{
    surface()->repaintOutputs();

    if (!changes.has(StateChanged))
        return;

    const CZBitset<State> stateChanges { state() ^ prevAtoms.state };

    if (stateChanges.has(Maximized))
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
        else if (!moveSession().isActive())
        {
            surface()->setPos(prevRect.pos());
        }
    }

    if (stateChanges.has(Fullscreen))
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

    if (stateChanges.has(Activated) && activated())
    {
        surface()->setMinimized(false);
        surface()->raise();
    }

    if (stateChanges.has(Fullscreen | Maximized) && !pendingConfiguration().state.has(Fullscreen | Maximized))
        setExclusiveOutput(nullptr);
}
//! [atomsChanged]

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
    LOutput *output { cursor()->output() };

    if (!output || maximized())
        return;

    if (!fullscreen())
        prevRect = LRect(surface()->pos(), windowGeometry().size());

    const LSize extraGeoSize {
        extraGeometry().left + extraGeometry().right,
        extraGeometry().top + extraGeometry().bottom };

    if (prevRect.area() == 0)
    {
        prevRect.setSize(output->availableGeometry().size() - extraGeoSize);
        prevRect.setPos(output->pos() + output->availableGeometry().pos());
    }

    setExclusiveOutput(output);
    configureSize(output->availableGeometry().size() - extraGeoSize);
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

    if (prevRect.area() == 0)
    {
        prevRect.setSize(output->availableGeometry().size() - LSize(extraGeometry().left + extraGeometry().right, extraGeometry().top + extraGeometry().bottom));
        prevRect.setPos(output->pos() + output->availableGeometry().pos());
    }

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
    configureState(pendingConfiguration().state & ~Activated);
    surface()->setMinimized(true);

    if (surface()->hasPointerFocus())
        seat()->pointer()->setFocus(nullptr);

    if (surface()->hasKeyboardFocus())
        seat()->keyboard()->setFocus(nullptr);

    moveSession().stop();
    resizeSession().stop();
}
//! [setMinimizedRequest]

//! [unsetMinimizedRequest]
void LToplevelRole::unsetMinimizedRequest()
{
    /* This request is always triggered by a foreign client */

    surface()->setMinimized(false);
}
//! [unsetMinimizedRequest]

//! [activateRequest]
void LToplevelRole::activateRequest()
{
    /* This request is always triggered by a foreign client or
     * by the default implementation of LActivationTokenManager::activateSurfaceRequest() */

    configureState(pendingConfiguration().state | Activated);
}
//! [activateRequest]

//! [closeRequest]
void LToplevelRole::closeRequest()
{
    /* This request is always triggered by a foreign client */

    close();
}
//! [closeRequest]

//! [foreignControllerFilter]
bool LToplevelRole::foreignControllerFilter(Protocols::ForeignToplevelManagement::GForeignToplevelManager *manager)
{
    L_UNUSED(manager)

    /* Allow all foreign clients to control the toplevel */
    return true;
}
//! [foreignControllerFilter]

//! [foreignHandleFilter]
bool LToplevelRole::foreignHandleFilter(Protocols::ForeignToplevelList::GForeignToplevelList *foreignList)
{
    L_UNUSED(foreignList)

    /* Allow all clients to get a handle for this toplevel */
    return true;
}
//! [foreignHandleFilter]

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
