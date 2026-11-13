#include <functional>
#include <CZ/Ream/RPainter.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureFrame.h>
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

//! [foreignParentFilter]
bool LToplevelRole::foreignParentFilter(LToplevelRole *parent)
{
    CZ_UNUSED(parent)

    /* Allow any client to parent its toplevels to exported ones (e.g. portal dialogs) */
    return true;
}
//! [foreignParentFilter]

//! [imageCaptureFrameRequest]
void LToplevelRole::imageCaptureFrameRequest(LImageCaptureFrame *frame)
{
    auto target { frame->surface() };

    if (!target)
    {
        frame->fail(LImageCaptureFrame::Unknown);
        return;
    }

    const SkRegion region { frame->paintRegion() };

    if (!region.isEmpty())
    {
        auto pass { target->beginPass(RPassCap_Painter) };

        if (!pass)
        {
            frame->fail(LImageCaptureFrame::Unknown);
            return;
        }

        auto *painter { pass->getPainter() };

        /* Transparent where nothing is drawn */
        painter->setBlendMode(RBlendMode::Src);
        painter->setColor(SK_ColorTRANSPARENT);
        painter->drawColor(region);
        painter->setBlendMode(RBlendMode::SrcOver);

        /* Surface coordinates relative to the window geometry, to buffer pixels */
        const SkIPoint origin { -windowGeometry().x(), -windowGeometry().y() };
        const SkISize geometry { windowGeometry().isEmpty() ? surface()->size() : windowGeometry().size() };
        const SkScalar sx { SkScalar(frame->session().bufferSize().width()) / SkScalar(std::max(1, geometry.width())) };
        const SkScalar sy { SkScalar(frame->session().bufferSize().height()) / SkScalar(std::max(1, geometry.height())) };

        /* Back to front: subsurfaces below, the surface, subsurfaces above */
        const std::function<void(LSurface*, SkIPoint)> draw = [&](LSurface *s, SkIPoint pos)
        {
            for (auto *sub : s->subsurfacesBelow())
                if (sub->surface()->mapped())
                    draw(sub->surface(), pos + sub->localPos());

            if (auto image { s->image() })
            {
                RDrawImageInfo info {};
                info.image = image;
                info.src = s->srcRect();
                info.srcScale = s->scale();
                info.srcTransform = s->bufferTransform();
                info.dst = SkIRect::MakeLTRB(
                    SkScalarRoundToInt(pos.x() * sx),
                    SkScalarRoundToInt(pos.y() * sy),
                    SkScalarRoundToInt((pos.x() + s->size().width()) * sx),
                    SkScalarRoundToInt((pos.y() + s->size().height()) * sy));
                info.minFilter = info.magFilter = RImageFilter::Linear;
                painter->drawImage(info, &region);
            }

            for (auto *sub : s->subsurfacesAbove())
                if (sub->surface()->mapped())
                    draw(sub->surface(), pos + sub->localPos());
        };

        draw(surface(), origin);
    }

    frame->ready();
}
//! [imageCaptureFrameRequest]

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
