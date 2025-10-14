#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Cursor/LCursor.h>

using namespace CZ;

//! [rolePos]
SkIPoint LToplevelRole::rolePos() const
{
    return surface()->pos() - windowGeometry().topLeft();
}
//! [rolePos]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest(const CZEvent &triggeringEvent)
{
    if (isFullscreen())
        return;

    LOutput *activeOutput { cursor()->output() };

    if (triggeringEvent.isTouchEvent())
    {
        if (!triggeringEvent.typeIsAnyOf(CZEvent::Type::TouchDown))
            return;

        if (!activeOutput)
            return;

        const auto &touchDownEvent { static_cast<const CZTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id) };

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

        if (triggeringEvent.isPointerEvent())
        {
            if (!triggeringEvent.typeIsAnyOf(CZEvent::Type::PointerButton))
                return;

            const auto &pointerButtonEvent { static_cast<const CZPointerButtonEvent&>(triggeringEvent) };

            if (!seat()->pointer()->isButtonPressed(pointerButtonEvent.button))
                return;
        }

        moveSession().start(triggeringEvent, SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
    }
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(const CZEvent &triggeringEvent, CZBitset<CZEdge> edge)
{
    if (isFullscreen())
        return;

    LOutput *activeOutput { cursor()->output() };

    if (triggeringEvent.isTouchEvent())
    {
        if (!triggeringEvent.typeIsAnyOf(CZEvent::Type::TouchDown))
            return;

        if (!activeOutput)
            return;

        const auto &touchDownEvent { static_cast<const CZTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id) };

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
        if (triggeringEvent.isPointerEvent())
        {
            if (!triggeringEvent.typeIsAnyOf(CZEvent::Type::PointerButton))
                return;

            const auto &pointerButtonEvent { static_cast<const CZPointerButtonEvent&>(triggeringEvent) };

            if (!seat()->pointer()->isButtonPressed(pointerButtonEvent.button))
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
        configureBounds(output->availableGeometry().size());
    }
    else
        configureBounds(0, 0);

    configureSize(0,0);
    configureState(pendingConfiguration().windowState | CZWinActivated);
    configureDecorationMode(ClientSide);
    configureCapabilities(WindowMenuCap | FullscreenCap | MaximizeCap | MinimizeCap);
}
//! [configureRequest]


//! [atomsChanged]
void LToplevelRole::stateChanged(CZBitset<Changes> changes, const State &prev)
{
    surface()->repaintOutputs();

    if (!changes.has(StateChanged))
        return;

    const CZBitset<CZWindowState> stateChanges { windowState() ^ prev.windowState };

    if (stateChanges.has(CZWinMaximized))
    {
        if (isMaximized())
        {
            if (exclusiveOutput())
            {
                surface()->raise();
                surface()->setPos(exclusiveOutput()->pos() + exclusiveOutput()->availableGeometry().topLeft());
                setMinimized(false);
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().windowState & ~CZWinMaximized);
            }
        }
        else if (!moveSession().isActive())
        {
            surface()->setPos(prevRect.topLeft());
        }
    }

    if (stateChanges.has(CZWinFullscreen))
    {
        if (isFullscreen())
        {
            if (exclusiveOutput())
            {
                surface()->setPos(exclusiveOutput()->pos());
                surface()->raise();
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().windowState & ~CZWinFullscreen);
            }
        }
        else
            surface()->setPos(prevRect.topLeft());
    }

    if (stateChanges.has(CZWinActivated) && isActivated())
    {
        setMinimized(false);
        surface()->raise();
    }

    if (stateChanges.has(CZWinFullscreen | CZWinMaximized) && !pendingConfiguration().windowState.has(CZWinFullscreen | CZWinMaximized))
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

    if (!output || isMaximized())
        return;

    if (!isFullscreen())
        prevRect = SkIRect::MakePtSize(surface()->pos(), windowGeometry().size());

    if (prevRect.width() == 0 || prevRect.height() == 0)
    {
        prevRect.setXYWH(
            output->pos().x() + output->availableGeometry().x(),
            output->pos().y() + output->availableGeometry().y(),
            output->availableGeometry().width(),
            output->availableGeometry().height());
    }

    setExclusiveOutput(output);
    configureSize(output->availableGeometry().size());
    configureState(CZWinActivated | CZWinMaximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    if (!isMaximized())
        return;

    configureState(pendingConfiguration().windowState &~ CZWinMaximized);
    configureSize(prevRect.size());
}
//! [unsetMaximizedRequest]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *preferredOutput)
{
    LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output || isFullscreen())
        return;

    if (!isMaximized())
        prevRect = SkIRect::MakePtSize(surface()->pos(), windowGeometry().size());

    if (prevRect.width() == 0 || prevRect.height() == 0)
    {
        prevRect.setXYWH(
            output->pos().x() + output->availableGeometry().x(),
            output->pos().y() + output->availableGeometry().y(),
            output->availableGeometry().width(),
            output->availableGeometry().height());
    }

    setExclusiveOutput(output);
    configureSize(output->size());
    configureState(CZWinActivated | CZWinFullscreen);
}
//! [setFullscreenRequest]

//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    if (!isFullscreen())
        return;

    configureState(pendingConfiguration().windowState &~ CZWinFullscreen);
    configureSize(prevRect.size());
}
//! [unsetFullscreenRequest]

//! [setMinimizedRequest]
void LToplevelRole::setMinimizedRequest()
{
    setMinimized(true);
}
//! [setMinimizedRequest]

//! [unsetMinimizedRequest]
void LToplevelRole::unsetMinimizedRequest()
{
    /* This request is always triggered by a foreign client */

    setMinimized(false);
}
//! [unsetMinimizedRequest]

//! [activateRequest]
void LToplevelRole::activateRequest()
{
    /* This request is always triggered by a foreign client or
     * by the default implementation of LActivationTokenManager::activateSurfaceRequest() */

    configureState(pendingConfiguration().windowState | CZWinActivated);
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
    CZ_UNUSED(manager)

    /* Allow all foreign clients to control the toplevel */
    return true;
}
//! [foreignControllerFilter]

//! [foreignHandleFilter]
bool LToplevelRole::foreignHandleFilter(Protocols::ForeignToplevelList::GForeignToplevelList *foreignList)
{
    CZ_UNUSED(foreignList)

    /* Allow all clients to get a handle for this toplevel */
    return true;
}
//! [foreignHandleFilter]

//! [showWindowMenuRequest]
void LToplevelRole::showWindowMenuRequest(const CZEvent &triggeringEvent, Int32 x, Int32 y)
{
    CZ_UNUSED(triggeringEvent);
    CZ_UNUSED(x);
    CZ_UNUSED(y);

    /* Here the compositor should render a context menu showing
     * the minimize, maximize and fullscreen options */
}
//! [showWindowMenuRequest]

void LToplevelRole::minimizedChanged()
{
    if (isMinimized())
    {
        configureState(pendingConfiguration().windowState & ~CZWinActivated);

        if (surface()->hasPointerFocus())
            seat()->pointer()->setFocus(nullptr);

        if (surface()->hasKeyboardFocus())
            seat()->keyboard()->setFocus(nullptr);

        moveSession().stop();
        resizeSession().stop();
    }
}
