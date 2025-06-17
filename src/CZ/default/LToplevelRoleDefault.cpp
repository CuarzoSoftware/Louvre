#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LTouchPoint.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LCursor.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LToplevelRole::rolePos() const
{
    return surface()->pos() - windowGeometry().topLeft() + SkIPoint(extraGeometry().left, extraGeometry().top);
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

        const SkPoint initDragPoint { LTouch::toGlobal(activeOutput, touchPoint->pos()) };
        moveSession().start(triggeringEvent, SkIPoint::Make(initDragPoint.x(), initDragPoint.y()));
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

        moveSession().start(triggeringEvent, SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
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

        const SkPoint initDragPoint { LTouch::toGlobal(activeOutput, touchPoint->pos()) };

        resizeSession().start(triggeringEvent, edge, SkIPoint(initDragPoint.x(), initDragPoint.y()));
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

        resizeSession().start(triggeringEvent, edge, SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
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
            SkISize(
                output->availableGeometry().width() - extraGeometry().left - extraGeometry().right,
                output->availableGeometry().height() - extraGeometry().top - extraGeometry().bottom));
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
                surface()->setPos(exclusiveOutput()->pos() + exclusiveOutput()->availableGeometry().topLeft());
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
            surface()->setPos(prevRect.topLeft());
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
            surface()->setPos(prevRect.topLeft());
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
        prevRect = SkIRect::MakePtSize(surface()->pos(), windowGeometry().size());

    const SkISize extraGeoSize {
        extraGeometry().left + extraGeometry().right,
        extraGeometry().top + extraGeometry().bottom };

    if (prevRect.width() == 0 || prevRect.height() == 0)
    {
        prevRect.setXYWH(
            output->pos().x() + output->availableGeometry().x(),
            output->pos().y() + output->availableGeometry().y(),
            output->availableGeometry().width() - extraGeoSize.width(),
            output->availableGeometry().height() - extraGeoSize.height());
    }

    setExclusiveOutput(output);
    configureSize(
        output->availableGeometry().width() - extraGeoSize.width(),
        output->availableGeometry().height() - extraGeoSize.height());
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
        prevRect = SkIRect::MakePtSize(surface()->pos(), windowGeometry().size());

    if (prevRect.width() == 0 || prevRect.height() == 0)
    {
        prevRect.setXYWH(
            output->pos().x() + output->availableGeometry().x(),
            output->pos().y() + output->availableGeometry().y(),
            output->availableGeometry().width() - extraGeometry().left - extraGeometry().right,
            output->availableGeometry().height() - extraGeometry().top - extraGeometry().bottom);
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
