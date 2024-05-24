#include <LSessionLockManager.h>
#include <private/LScenePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSceneTouchPoint.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LSurfaceView.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LDND.h>
#include <LDNDIconRole.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LLauncher.h>
#include <LClient.h>
#include <LTouch.h>
#include <LTouchPoint.h>
#include <LUtils.h>
#include <unistd.h>

using LVS = LView::LViewState;
using LSS = LScene::LScenePrivate::State;

LScene::LScene() : LPRIVATE_INIT_UNIQUE(LScene)
{
    imp()->view.setPos(0);
    LView *baseView = &imp()->view;
    baseView->m_scene = this;
    baseView->m_state.add(LVS::IsScene);
}

LScene::~LScene() {}

const std::vector<LView *> &LScene::pointerFocus() const
{
    return imp()->pointerFocus;
}

const std::vector<LView *> &LScene::keyboardFocus() const
{
    return imp()->keyboardFocus;
}

const std::vector<LSceneTouchPoint *> &LScene::touchPoints() const
{
    return imp()->touchPoints;
}

LSceneTouchPoint *LScene::findTouchPoint(Int32 id) const
{
    for (LSceneTouchPoint *tp : imp()->touchPoints)
        if (id == tp->id())
            return tp;
    return nullptr;
}

void LScene::handleInitializeGL(LOutput *output)
{
    if (!output)
        return;

    imp()->mutex.lock();
    imp()->view.m_fb = output->framebuffer();
    imp()->mutex.unlock();
}

void LScene::handlePaintGL(LOutput *output)
{
    if (!output)
        return;

    imp()->mutex.lock();
    imp()->view.m_fb = output->framebuffer();
    imp()->view.render();
    imp()->mutex.unlock();
}

void LScene::handleMoveGL(LOutput *output)
{
    if (!output)
        return;

    imp()->mutex.lock();
    imp()->view.m_fb = output->framebuffer();
    imp()->view.damageAll(output);
    imp()->mutex.unlock();
}

void LScene::handleResizeGL(LOutput *output)
{
    if (!output)
        return;

    imp()->mutex.lock();
    imp()->view.m_fb = output->framebuffer();
    imp()->view.damageAll(output);
    imp()->mutex.unlock();
}

void LScene::handleUninitializeGL(LOutput *output)
{
    if (!output)
        return;

    imp()->mutex.lock();
    auto it { imp()->view.m_sceneThreadsMap.find(output->threadId()) };
    if (it != imp()->view.m_sceneThreadsMap.end())
        imp()->view.m_sceneThreadsMap.erase(it);
    imp()->mutex.unlock();
}

void LScene::handlePointerMoveEvent(const LPointerMoveEvent &event, LBitset<EventOptions> options)
{
    // Prevent recursive calls
    if (imp()->state.check(LSS::HandlingPointerMoveEvent))
        return;

    imp()->currentPointerMoveEvent = event;

    imp()->currentPointerEnterEvent.setDevice(event.device());
    imp()->currentPointerEnterEvent.setMs(event.ms());
    imp()->currentPointerEnterEvent.setUs(event.us());
    imp()->currentPointerEnterEvent.setSerial(event.serial());

    imp()->currentPointerLeaveEvent.setDevice(event.device());
    imp()->currentPointerLeaveEvent.setMs(event.ms());
    imp()->currentPointerLeaveEvent.setUs(event.us());
    imp()->currentPointerLeaveEvent.setSerial(event.serial());

    cursor()->move(event.delta());

    LWeak<LSurface> surface;
    LWeak<LSurfaceView> firstSurfaceView;

    if ((options & (WaylandEvents | PointerConstraints)) == (WaylandEvents | PointerConstraints) && seat()->pointer()->focus() && seat()->pointer()->focus()->imp()->lastPointerEventView)
    {
        surface.reset(seat()->pointer()->focus());
        firstSurfaceView.reset(seat()->pointer()->focus()->imp()->lastPointerEventView);

        const LPointF pos { firstSurfaceView->pos() };
        LPointF scalingVector { 1.f, 1.f };

        if ((firstSurfaceView->scalingEnabled() || firstSurfaceView->parentScalingEnabled()) && firstSurfaceView->scalingVector().area() != 0.f)
            scalingVector = firstSurfaceView->scalingVector();

        const LPointF scaledPosDiff { (cursor()->pos() - pos) / scalingVector };

        if (surface->pointerConstraintMode() != LSurface::PointerConstraintMode::Free)
        {
            if (surface->pointerConstraintRegion().containsPoint(scaledPosDiff))
                surface->enablePointerConstraint(true);
        }

        if (surface->pointerConstraintEnabled())
        {
            if (surface->pointerConstraintMode() == LSurface::PointerConstraintMode::Lock)
            {
                if (surface->lockedPointerPosHint().x() >= 0.f)
                    cursor()->setPos(pos + surface->lockedPointerPosHint() * scalingVector);
                else
                {
                    cursor()->move(-event.delta().x(), -event.delta().y());

                    const LPointF closestPoint {
                        surface->pointerConstraintRegion().closestPointFrom((cursor()->pos() - pos) / scalingVector, 0.5f) * scalingVector
                    };

                    cursor()->setPos(pos + closestPoint);
                }
            }
            else
            {
                const LPointF closestPoint {
                    surface->pointerConstraintRegion().closestPointFrom(scaledPosDiff, 0.5f) * scalingVector
                };

                cursor()->setPos(pos + closestPoint);
            }
        }
        else
        {
            surface.reset();
            firstSurfaceView.reset();
        }
    }

    cursor()->repaintOutputs(true);

    imp()->state.remove(LSS::ChildrenListChanged | LSS::PointerIsBlocked);
    imp()->state.add(LSS::HandlingPointerMoveEvent);
    LView::removeFlagWithChildren(mainView(), LVS::PointerMoveDone);
    imp()->handlePointerMove(mainView());
    imp()->state.remove(LSS::HandlingPointerMoveEvent);

    if (!(options & WaylandEvents))
        return;

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    LPointF localPos;

    if (!firstSurfaceView || !surface || !surface->hasPointerFocus() || !surface->pointerConstraintEnabled())
    {
        for (LView *view : pointerFocus())
        {
            if (view->type() == LView::Surface)
            {
                firstSurfaceView = static_cast<LSurfaceView*>(view);
                break;
            }
        }
    }

    if (firstSurfaceView)
    {
        if (!sessionLocked || (sessionLockManager()->client() == firstSurfaceView->surface()->client()))
        {
            localPos = imp()->viewLocalPos(firstSurfaceView , cursor()->pos());
            surface = firstSurfaceView->surface();
        }
    }

    const bool activeDND { seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch };

    if (activeDND)
    {
        if (seat()->dnd()->icon())
        {
            seat()->dnd()->icon()->surface()->setPos(cursor()->pos());
            seat()->dnd()->icon()->surface()->repaintOutputs();
        }

        seat()->pointer()->setDraggingSurface(nullptr);
        seat()->pointer()->setFocus(nullptr);
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() != LEvent::Type::Touch)
        {
            activeResizing = true;
            session->updateDragPoint(cursor()->pos());
        }
    }

    if (activeResizing)
        return;

    bool activeMoving { false };

    for (LToplevelMoveSession *session : seat()->toplevelMoveSessions())
    {
        if (session->triggeringEvent().type() != LEvent::Type::Touch)
        {
            activeMoving = true;
            session->updateDragPoint(cursor()->pos());
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->maximized())
                session->toplevel()->configureState(session->toplevel()->pending().state &~ LToplevelRole::Maximized);
        }
    }

    if (activeMoving)
        return;

    // If a surface has the left pointer button held down
    if (seat()->pointer()->draggingSurface())
    {
        if (seat()->pointer()->draggingSurface()->imp()->lastPointerEventView)
        {
            imp()->currentPointerMoveEvent.localPos = imp()->viewLocalPos(seat()->pointer()->draggingSurface()->imp()->lastPointerEventView, cursor()->pos());
            seat()->pointer()->sendMoveEvent(imp()->currentPointerMoveEvent);
        }
        else
        {
            imp()->currentPointerMoveEvent.localPos = cursor()->pos() - seat()->pointer()->draggingSurface()->rolePos();
            seat()->pointer()->sendMoveEvent(imp()->currentPointerMoveEvent);
        }

        return;
    }

    if (surface)
    {
        surface->imp()->lastPointerEventView.reset(firstSurfaceView);

        if (activeDND)
        {
            if (seat()->dnd()->focus() == surface)
                seat()->dnd()->sendMoveEvent(localPos, event.ms());
            else
                seat()->dnd()->setFocus(surface, localPos);
        }
        else
        {
            if (seat()->pointer()->focus() == surface)
            {
                imp()->currentPointerMoveEvent.localPos = localPos;
                seat()->pointer()->sendMoveEvent(imp()->currentPointerMoveEvent);
            }
            else
                seat()->pointer()->setFocus(surface, localPos);
        }
    }
    else
    {
        if (activeDND)
            seat()->dnd()->setFocus(nullptr, LPointF());
        else
            seat()->pointer()->setFocus(nullptr);
    }
}

void LScene::handlePointerButtonEvent(const LPointerButtonEvent &event, LBitset<EventOptions> options)
{
    // Prevent recursive calls
    if (imp()->state.check(LSS::HandlingPointerButtonEvent))
        return;

    imp()->state.add(LSS::HandlingPointerButtonEvent);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerButtonDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerButtonDone))
            continue;

        view->m_state.add(LVS::PointerButtonDone);
        view->pointerButtonEvent(event);

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(LSS::HandlingPointerButtonEvent);

    if (!(options & WaylandEvents))
        return;

    LPointer &pointer { *seat()->pointer() };
    LKeyboard &keyboard { *seat()->keyboard() };
    LDND &dnd{ *seat()->dnd() };

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    const bool activeDND { dnd.dragging() && dnd.triggeringEvent().type() != LEvent::Type::Touch };

    if (activeDND)
    {
        if (event.state() == LPointerButtonEvent::Released && event.button() == LPointerButtonEvent::Left)
            dnd.drop();

        keyboard.setFocus(nullptr);
        pointer.setFocus(nullptr);
        pointer.setDraggingSurface(nullptr);
        return;
    }

    if (!pointer.focus())
    {
        LSurface *surface { nullptr };
        LView *view { viewAt(cursor()->pos(), LView::Undefined, InputFilter::Pointer) };

        if (view && view->type() == LView::Surface)
            surface = static_cast<LSurfaceView*>(view)->surface();

        if (surface)
        {
            if (sessionLocked && sessionLockManager()->client() != surface->client())
                return;

            keyboard.setFocus(surface);
            pointer.setFocus(surface, imp()->viewLocalPos(view, cursor()->pos()));
            pointer.sendButtonEvent(event);

            if (!surface->popup())
                seat()->dismissPopups();
        }
        else
        {
            keyboard.setFocus(nullptr);
            seat()->dismissPopups();
        }

        return;
    }

    if (event.button() != LPointerButtonEvent::Left)
    {
        pointer.sendButtonEvent(event);
        return;
    }

    // Left button pressed
    if (event.state() == LPointerButtonEvent::Pressed)
    {
        // We save the pointer focus surface to continue sending events to it even when the cursor
        // is outside of it (while the left button is being held down)
        pointer.setDraggingSurface(seat()->pointer()->focus());

        if (!keyboard.focus() || !pointer.focus()->isSubchildOf(keyboard.focus()))
            keyboard.setFocus(seat()->pointer()->focus());

        if (pointer.focus()->toplevel() && !pointer.focus()->toplevel()->activated())
            pointer.focus()->toplevel()->configureState(pointer.focus()->toplevel()->pending().state | LToplevelRole::Activated);

        if (!pointer.focus()->popup())
            seat()->dismissPopups();

        pointer.sendButtonEvent(event);

        if (pointer.focus() == compositor()->surfaces().back())
            return;

        if (pointer.focus()->parent())
            pointer.focus()->topmostParent()->raise();
        else
            pointer.focus()->raise();
    }
    // Left button released
    else
    {
        pointer.sendButtonEvent(event);

        // Stop pointer toplevel resizing sessions
        for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
        {
            if ((*it)->triggeringEvent().type() != LEvent::Type::Touch)
                it = (*it)->stop();
            else
                it++;
        }

        // Stop pointer toplevel moving sessions
        for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
        {
            if ((*it)->triggeringEvent().type() != LEvent::Type::Touch)
                it = (*it)->stop();
            else
                it++;
        }

        // We stop sending events to the surface on which the left button was being held down
        pointer.setDraggingSurface(nullptr);

        if (pointer.focus()->imp()->lastPointerEventView)
        {
            if (!imp()->pointIsOverView(pointer.focus()->imp()->lastPointerEventView, cursor()->pos(), InputFilter::Pointer))
                pointer.setFocus(nullptr);
        }
        else
            if (!pointer.focus()->inputRegion().containsPoint(cursor()->pos() - pointer.focus()->pos()))
                pointer.setFocus(nullptr);
    }
}

void LScene::handlePointerScrollEvent(const LPointerScrollEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerScrollEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerScrollDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerScrollDone))
                continue;

        view->m_state.add(LVS::PointerScrollDone);
        view->pointerScrollEvent(event);

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendScrollEvent(event);
}

void LScene::handlePointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerSwipeBeginEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->pointerSwipeEndEvent.setFingers(event.fingers());
    imp()->pointerSwipeEndEvent.setDevice(event.device());

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerSwipeBeginDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerSwipeBeginDone))
            continue;

        view->m_state.add(LVS::PointerSwipeBeginDone);

        if (!view->m_state.check(LVS::PendingSwipeEnd))
        {
            view->m_state.add(LVS::PendingSwipeEnd);
            view->pointerSwipeBeginEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendSwipeBeginEvent(event);
}

void LScene::handlePointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerSwipeUpdateEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerSwipeUpdateDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerSwipeUpdateDone))
            continue;

        view->m_state.add(LVS::PointerSwipeUpdateDone);

        if (view->m_state.check(LVS::PendingSwipeEnd))
            view->pointerSwipeUpdateEvent(event);

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendSwipeUpdateEvent(event);
}

void LScene::handlePointerSwipeEndEvent(const LPointerSwipeEndEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerSwipeEndEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerSwipeEndDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerSwipeEndDone))
            continue;

        view->m_state.add(LVS::PointerSwipeEndDone);

        if (view->m_state.check(LVS::PendingSwipeEnd))
        {
            view->m_state.remove(LVS::PendingSwipeEnd);
            view->pointerSwipeEndEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendSwipeEndEvent(event);
}

void LScene::handlePointerPinchBeginEvent(const LPointerPinchBeginEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerPinchBeginEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->pointerPinchEndEvent.setFingers(event.fingers());
    imp()->pointerPinchEndEvent.setDevice(event.device());

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerPinchBeginDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerPinchBeginDone))
            continue;

        view->m_state.add(LVS::PointerPinchBeginDone);

        if (!view->m_state.check(LVS::PendingPinchEnd))
        {
            view->m_state.add(LVS::PendingPinchEnd);
            view->pointerPinchBeginEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendPinchBeginEvent(event);
}

void LScene::handlePointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerPinchUpdateEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerPinchUpdateDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerPinchUpdateDone))
            continue;

        view->m_state.add(LVS::PointerPinchUpdateDone);

        if (view->m_state.check(LVS::PendingPinchEnd))
            view->pointerPinchUpdateEvent(event);

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendPinchUpdateEvent(event);
}

void LScene::handlePointerPinchEndEvent(const LPointerPinchEndEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerPinchEndEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerPinchEndDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerPinchEndDone))
            continue;

        view->m_state.add(LVS::PointerPinchEndDone);

        if (view->m_state.check(LVS::PendingPinchEnd))
        {
            view->m_state.remove(LVS::PendingPinchEnd);
            view->pointerPinchEndEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendPinchEndEvent(event);
}

void LScene::handlePointerHoldBeginEvent(const LPointerHoldBeginEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerHoldBeginEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->pointerHoldEndEvent.setFingers(event.fingers());
    imp()->pointerHoldEndEvent.setDevice(event.device());

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerHoldBeginDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerHoldBeginDone))
            continue;

        view->m_state.add(LVS::PointerHoldBeginDone);

        if (!view->m_state.check(LVS::PendingHoldEnd))
        {
            view->m_state.add(LVS::PendingHoldEnd);
            view->pointerHoldBeginEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendHoldBeginEvent(event);
}

void LScene::handlePointerHoldEndEvent(const LPointerHoldEndEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingPointerHoldEndEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->pointerFocus)
        view->m_state.remove(LVS::PointerHoldEndDone);

retry:

    imp()->state.remove(LSS::PointerFocusVectorChanged);

    for (LView *view : imp()->pointerFocus)
    {
        if (view->m_state.check(LVS::PointerHoldEndDone))
            continue;

        view->m_state.add(LVS::PointerHoldEndDone);

        if (view->m_state.check(LVS::PendingHoldEnd))
        {
            view->m_state.remove(LVS::PendingHoldEnd);
            view->pointerHoldEndEvent(event);
        }

        if (imp()->state.check(LSS::PointerFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (!(options & WaylandEvents))
        return;

    seat()->pointer()->sendHoldEndEvent(event);
}

void LScene::handleKeyboardKeyEvent(const LKeyboardKeyEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingKeyboardKeyEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->keyboardFocus)
        view->m_state.remove(LVS::KeyDone);

retry:

    imp()->state.remove(LSS::KeyboardFocusVectorChanged);

    for (LView *view : imp()->keyboardFocus)
    {
        if (view->m_state.check(LVS::KeyDone))
            continue;

        view->m_state.add(LVS::KeyDone);
        view->keyEvent(event);

        if (imp()->state.check(LSS::KeyboardFocusVectorChanged))
            goto retry;
    }

    imp()->state.remove(handlingEventFlag);

    if (options & WaylandEvents)
        seat()->keyboard()->sendKeyEvent(event);

    if (!(options & AuxFunc))
        return;

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    LKeyboard &keyboard { *seat()->keyboard() };
    const bool L_CTRL      { keyboard .isKeyCodePressed(KEY_LEFTCTRL) };
    const bool L_SHIFT     { keyboard .isKeyCodePressed(KEY_LEFTSHIFT) };
    const bool mods        { keyboard .isKeyCodePressed(KEY_LEFTALT) && L_CTRL };
    const xkb_keysym_t sym { keyboard .keySymbol(event.keyCode()) };

    if (event.state() == LKeyboardKeyEvent::Released)
    {
        if (event.keyCode() == KEY_ESC && L_CTRL && L_SHIFT)
        {
            compositor()->finish();
            return;
        }
        else if (L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::NoAction);

        if (sessionLocked)
            return;

        if (event.keyCode() == KEY_F1 && !mods)
            LLauncher::launch("weston-terminal");
        else if (L_CTRL && (sym == XKB_KEY_q || sym == XKB_KEY_Q))
        {
            if (keyboard.focus())
                keyboard.focus()->client()->destroyLater();
        }
        else if (L_CTRL && (sym == XKB_KEY_m || sym == XKB_KEY_M))
        {
            if (keyboard.focus() && keyboard.focus()->toplevel() && !keyboard.focus()->toplevel()->fullscreen())
                keyboard.focus()->setMinimized(true);
        }
        // Screenshot
        else if (L_CTRL && L_SHIFT && event.keyCode() == KEY_3)
        {
            if (cursor()->output() && cursor()->output()->bufferTexture(0))
            {
                std::filesystem::path path { getenvString("HOME") };

                if (path.empty())
                    return;

                path /= "Desktop/Louvre_Screenshoot_";

                char timeString[32];
                const auto now { std::chrono::system_clock::now() };
                const auto time { std::chrono::system_clock::to_time_t(now) };
                std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S.png", std::localtime(&time));

                path += timeString;

                cursor()->output()->bufferTexture(0)->save(path);
            }
        }
    }

    // Key pressed
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop session
        if (L_CTRL)
            seat()->dnd()->setPreferredAction(LDND::Copy);

        // SHIFT sets the Move as the preferred action in drag & drop session
        else if (L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
    }
}

void LScene::handleTouchDownEvent(const LTouchDownEvent &event, const LPointF &globalPos, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingTouchEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    imp()->currentTouchPoint = findTouchPoint(event.id());

    if (!imp()->currentTouchPoint)
        imp()->currentTouchPoint = new LSceneTouchPoint(this, event);

    imp()->currentTouchPoint->m_pressed = true;
    imp()->currentTouchPoint->m_pos = event.pos();
    imp()->touchDownEvent = event;
    imp()->touchGlobalPos = globalPos;

    imp()->state.remove(LSS::ChildrenListChanged);
    imp()->state.remove(LSS::TouchIsBlocked);

    LView::removeFlagWithChildren(mainView(), LVS::TouchDownDone);
    imp()->handleTouchDown(mainView());

    if (!(options & WaylandEvents))
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    LTouch &touch { *seat()->touch() };
    LTouchPoint *tp { touch.createOrGetTouchPoint(event) };

    // Check if a surface was touched
    LSurfaceView *surfaceView { nullptr };

    for (LView *view : imp()->currentTouchPoint->views())
    {
        if (view->type() == LView::Surface)
        {
            surfaceView = static_cast<LSurfaceView*>(view);
            surfaceView->surface()->imp()->lastTouchEventView.reset(surfaceView);
            break;
        }
    }

    if (surfaceView)
    {
        if (sessionLocked && sessionLockManager()->client() != surfaceView->surface()->client())
            goto end;

        event.localPos = globalPos - surfaceView->pos();

        if (!seat()->keyboard()->focus() || !surfaceView->surface()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(surfaceView->surface());

        tp->sendDownEvent(event, surfaceView->surface());
        surfaceView->surface()->raise();
    }
    else
    {
        tp->sendDownEvent(event);
        seat()->dismissPopups();
    }

    end:
    imp()->state.remove(handlingEventFlag);
}

void LScene::handleTouchMoveEvent(const LTouchMoveEvent &event, const LPointF &globalPos, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingTouchEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->currentTouchPoint = findTouchPoint(event.id());

    if (!imp()->currentTouchPoint)
        goto skipViews;

    imp()->currentTouchPoint->m_pos = event.pos();
    imp()->touchGlobalPos = globalPos;
    imp()->state.remove(LSS::ChildrenListChanged | LSS::TouchIsBlocked);

    for (LView *view : imp()->currentTouchPoint->views())
        view->m_state.remove(LVS::TouchMoveDone);

retry:

    imp()->currentTouchPoint->m_listChanged = false;

    for (LView *view : imp()->currentTouchPoint->views())
    {
        if (view->m_state.check(LVS::TouchMoveDone))
            continue;

        view->m_state.add(LVS::TouchMoveDone);
        event.localPos = imp()->viewLocalPos(view, globalPos);
        view->touchMoveEvent(event);

        if (imp()->currentTouchPoint->m_listChanged)
            goto retry;
    }

skipViews:

    if (!(options & WaylandEvents))
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    LTouchPoint *tp { seat()->touch()->findTouchPoint(event.id()) };

    if (!tp)
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    // Handle DND session
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch && dnd.triggeringEvent().subtype() == LEvent::Subtype::Down)
    {
        const auto &touchDownEvent { static_cast<const LTouchDownEvent&>(dnd.triggeringEvent()) };

        if (touchDownEvent.id() == tp->id())
        {
            if (dnd.icon())
            {
                dnd.icon()->surface()->setPos(globalPos);
                dnd.icon()->surface()->repaintOutputs();
            }

            LSurfaceView *surfaceView { static_cast<LSurfaceView*>(viewAt(globalPos, LView::Surface, InputFilter::Pointer | InputFilter::Touch)) };

            if (surfaceView)
            {
                if (dnd.focus() == surfaceView->surface())
                    dnd.sendMoveEvent(imp()->viewLocalPos(surfaceView, globalPos), event.ms());
                else
                    dnd.setFocus(surfaceView->surface(), imp()->viewLocalPos(surfaceView, globalPos));
            }
            else
                dnd.setFocus(nullptr, LPoint());
        }
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(session->triggeringEvent()) };

            if (touchDownEvent.id() == tp->id())
            {
                activeResizing = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configureState(session->toplevel()->pending().state &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeResizing)
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    bool activeMoving { false };

    for (LToplevelMoveSession *session : seat()->toplevelMoveSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(session->triggeringEvent()) };

            if (touchDownEvent.id() == tp->id())
            {
                activeMoving = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configureState(session->toplevel()->pending().state &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeMoving)
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    if (tp->surface())
    {
        if (tp->surface()->imp()->lastTouchEventView)
            event.localPos = imp()->viewLocalPos(tp->surface()->imp()->lastTouchEventView, globalPos);
        else
            event.localPos = globalPos - tp->surface()->rolePos();

        tp->sendMoveEvent(event);
    }
    else
        tp->sendMoveEvent(event);

    imp()->state.remove(handlingEventFlag);
}

void LScene::handleTouchUpEvent(const LTouchUpEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingTouchEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->currentTouchPoint = findTouchPoint(event.id());

    if (!imp()->currentTouchPoint)
        goto skipViews;

    imp()->currentTouchPoint->m_pressed = false;
    imp()->state.remove(LSS::ChildrenListChanged | LSS::TouchIsBlocked);

    for (LView *view : imp()->currentTouchPoint->views())
        view->m_state.remove(LVS::TouchUpDone);

retry:

    imp()->currentTouchPoint->m_listChanged = false;

    for (LView *view : imp()->currentTouchPoint->views())
    {
        if (view->m_state.check(LVS::TouchUpDone))
            continue;

        view->m_state.add(LVS::TouchUpDone);
        view->touchUpEvent(event);

        if (imp()->currentTouchPoint->m_listChanged)
            goto retry;
    }

skipViews:

    if (!(options & WaylandEvents))
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    LTouchPoint *tp { seat()->touch()->findTouchPoint(event.id()) };

    if (!tp)
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch && dnd.triggeringEvent().subtype() == LEvent::Subtype::Down)
    {
        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(dnd.triggeringEvent()) };

        if (touchDownEvent.id() == tp->id())
            dnd.drop();
    }

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &downEvent { static_cast<const LTouchDownEvent&>((*it)->triggeringEvent()) };

            if (downEvent.id() == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &downEvent { static_cast<const LTouchDownEvent&>((*it)->triggeringEvent()) };

            if (downEvent.id() == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

    tp->sendUpEvent(event);
    imp()->state.remove(handlingEventFlag);
}

void LScene::handleTouchFrameEvent(const LTouchFrameEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingTouchEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);
    imp()->state.remove(LSS::ChildrenListChanged | LSS::TouchIsBlocked);

    for (LView *view : imp()->currentTouchPoint->views())
        view->m_state.remove(LVS::TouchFrameDone);

retry:

    imp()->currentTouchPoint->m_listChanged = false;

    for (LView *view : imp()->currentTouchPoint->views())
    {
        if (view->m_state.check(LVS::TouchFrameDone))
            continue;

        view->m_state.add(LVS::TouchFrameDone);
        view->touchFrameEvent(event);

        if (imp()->currentTouchPoint->m_listChanged)
        {
            imp()->currentTouchPoint->m_listChanged = false;
            goto retry;
        }
    }

    for (auto it = imp()->touchPoints.begin(); it != imp()->touchPoints.end();)
    {
        if (!(*it)->pressed())
            it = (*it)->destroy();
        else
            it++;
    }

    if (!(options & WaylandEvents))
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    seat()->touch()->sendFrameEvent(event);
    imp()->state.remove(handlingEventFlag);
}

void LScene::handleTouchCancelEvent(const LTouchCancelEvent &event, LBitset<EventOptions> options)
{
    auto &&handlingEventFlag { LSS::HandlingTouchEvent };

    // Prevent recursive calls
    if (imp()->state.check(handlingEventFlag))
        return;

    imp()->state.add(handlingEventFlag);

    for (LView *view : imp()->currentTouchPoint->views())
        view->m_state.remove(LVS::TouchCancelDone);

retry:

    imp()->currentTouchPoint->m_listChanged = false;

    for (LView *view : imp()->currentTouchPoint->views())
    {
        if (view->m_state.check(LVS::TouchCancelDone))
            continue;

        view->m_state.add(LVS::TouchCancelDone);
        view->touchCancelEvent(event);

        if (imp()->currentTouchPoint->m_listChanged)
            goto retry;
    }

    while (!imp()->touchPoints.empty())
        imp()->touchPoints.back()->destroy();

    if (!(options & WaylandEvents))
    {
        imp()->state.remove(handlingEventFlag);
        return;
    }

    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch)
        dnd.drop();

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
            it = (*it)->stop();
        else
            it++;
    }

    // Stop touch toplevel moving sessions
    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
            it = (*it)->stop();
        else
            it++;
    }

    seat()->touch()->sendCancelEvent(event);
    imp()->state.remove(handlingEventFlag);
}

LSceneView *LScene::mainView() const
{
    return &imp()->view;
}

LView *LScene::viewAt(const LPoint &pos, LView::Type type, LBitset<InputFilter> filter)
{
    return imp()->viewAt(mainView(), pos, type, filter);
}
