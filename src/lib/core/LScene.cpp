#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSurfaceView.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LLauncher.h>
#include <LClient.h>
#include <unistd.h>

using LVS = LView::LViewPrivate::LViewState;

LScene::LScene() : LPRIVATE_INIT_UNIQUE(LScene)
{
    imp()->view.setPos(0);
    LView *baseView = &imp()->view;
    baseView->imp()->scene = this;
}

LScene::~LScene() {}

void LScene::handleInitializeGL(LOutput *output)
{
    imp()->mutex.lock();
    imp()->view.imp()->fb = output->framebuffer();
    imp()->mutex.unlock();
}

void LScene::handlePaintGL(LOutput *output)
{
    imp()->mutex.lock();
    imp()->view.imp()->fb = output->framebuffer();
    imp()->view.render();
    imp()->mutex.unlock();
}

void LScene::handleMoveGL(LOutput *output)
{
    imp()->mutex.lock();
    imp()->view.imp()->fb = output->framebuffer();
    imp()->view.damageAll(output);
    imp()->mutex.unlock();
}

void LScene::handleResizeGL(LOutput *output)
{
    imp()->mutex.lock();
    imp()->view.damageAll(output);
    imp()->mutex.unlock();
}

void LScene::handleUninitializeGL(LOutput *output)
{
    L_UNUSED(output);
    imp()->mutex.lock();
    auto it = imp()->view.imp()->threadsMap.find(output->threadId());

    if (it != imp()->view.imp()->threadsMap.end())
        imp()->view.imp()->threadsMap.erase(it);
    imp()->mutex.unlock();
}

LView *LScene::handlePointerMoveEvent(Float32 x, Float32 y, bool absolute, LPoint *outLocalPos)
{
    // Prevent recursive calls
    if (imp()->handlingPointerMove)
        return nullptr;

    LSurface *surface = nullptr;
    LView *view = nullptr;
    LPoint localPos;

    if (absolute)
        cursor()->setPos(x, y);
    else
        cursor()->move(x, y);

    imp()->listChanged = false;
    imp()->pointerIsBlocked = false;

    imp()->handlingPointerMove = true;
    LView::LViewPrivate::removeFlagWithChildren(mainView(), LVS::PointerMoveDone);
    imp()->handlePointerMove(mainView(), cursor()->pos(), &view);
    imp()->handlingPointerMove = false;

    if (view)
    {
        localPos = imp()->viewLocalPos(view, cursor()->pos());

        if (outLocalPos)
            *outLocalPos = localPos;

        if (view->type() == LView::Surface)
        {
            LSurfaceView *surfaceView = (LSurfaceView*)view;
            surface = surfaceView->surface();
        }
    }

    if (!handleWaylandPointerEventsEnabled())
        return view;

    // Repaint cursor outputs if hardware composition is not supported
    cursor()->repaintOutputs(true);

    // Update the drag & drop icon (if there was one)
    if (seat()->dndManager()->icon())
    {
        seat()->dndManager()->icon()->surface()->setPos(cursor()->pos());
        seat()->dndManager()->icon()->surface()->repaintOutputs();
    }

    // Update the toplevel size (if there was one being resized)
    if (seat()->pointer()->resizingToplevel())
    {
        seat()->pointer()->updateResizingToplevelSize(cursor()->pos());
        return view;
    }

    // Update the toplevel pos (if there was one being moved interactively)
    if (seat()->pointer()->movingToplevel())
    {
        seat()->pointer()->updateMovingToplevelPos(cursor()->pos());

        seat()->pointer()->movingToplevel()->surface()->repaintOutputs();

        if (seat()->pointer()->movingToplevel()->maximized())
            seat()->pointer()->movingToplevel()->configure(seat()->pointer()->movingToplevel()->pendingStates() &~ LToplevelRole::Maximized);

        return view;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if (seat()->dndManager()->dragging())
        seat()->pointer()->setDraggingSurface(nullptr);

    // If there was a surface holding the left pointer button
    if (seat()->pointer()->draggingSurface())
    {
        if (seat()->pointer()->draggingSurface()->imp()->lastPointerEventView)
            seat()->pointer()->sendMoveEvent(imp()->viewLocalPos(seat()->pointer()->draggingSurface()->imp()->lastPointerEventView, cursor()->pos()));
        else
            seat()->pointer()->sendMoveEvent();

        return view;
    }

    if (!surface)
    {
        seat()->pointer()->setFocus(nullptr);
    }
    else
    {
        surface->imp()->lastPointerEventView = (LSurfaceView*)view;

        if (seat()->pointer()->focus() == surface)
            seat()->pointer()->sendMoveEvent(localPos);
        else
            seat()->pointer()->setFocus(surface, localPos);
    }

    return view;
}

void LScene::handlePointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    // Prevent recursive calls
    if (imp()->handlingPointerButton)
        return;

    imp()->listChanged = false;
    imp()->handlingPointerButton = true;
    LView::LViewPrivate::removeFlagWithChildren(mainView(), LVS::PointerButtonDone);
    imp()->handlePointerButton(mainView(), button, state);
    imp()->handlingPointerButton = false;

    if (!handleWaylandPointerEventsEnabled())
        return;

    if (button == LPointer::Left && state == LPointer::Released)
        seat()->dndManager()->drop();

    if (!seat()->pointer()->focus())
    {
        LSurface *surface = nullptr;
        LView *view = viewAt(cursor()->pos());

        if (view && view->type() == LView::Surface)
        {
            LSurfaceView *surfaceView = (LSurfaceView*)view;
            surface = surfaceView->surface();
        }

        if (surface)
        {
            if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != surface->client())
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->dismissPopups();
            }

            if (!seat()->keyboard()->focus() || !surface->isSubchildOf(seat()->keyboard()->focus()))
                seat()->keyboard()->setFocus(surface);

            seat()->pointer()->setFocus(surface, imp()->viewLocalPos(view, cursor()->pos()));
            seat()->pointer()->sendButtonEvent(button, state);
        }
        // If no surface under the cursor
        else
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->pointer()->dismissPopups();
        }

        return;
    }

    seat()->pointer()->sendButtonEvent(button, state);

    if (button != LPointer::Button::Left)
        return;

    // Left button pressed
    if (state == LPointer::ButtonState::Pressed)
    {
        /* We save the pointer focus surface in order to continue sending events to it even when the cursor
         * is outside of it (while the left button is being held down)*/
        seat()->pointer()->setDraggingSurface(seat()->pointer()->focus());

        if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != seat()->pointer()->focus()->client())
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->pointer()->dismissPopups();
        }

        if (!seat()->pointer()->focus()->popup())
            seat()->pointer()->dismissPopups();

        if (!seat()->keyboard()->focus() || !seat()->pointer()->focus()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(seat()->pointer()->focus());

        if (seat()->pointer()->focus()->toplevel() && !seat()->pointer()->focus()->toplevel()->activated())
            seat()->pointer()->focus()->toplevel()->configure(seat()->pointer()->focus()->toplevel()->pendingStates() | LToplevelRole::Activated);

        // Raise surface
        if (seat()->pointer()->focus() == compositor()->surfaces().back())
            return;

        if (seat()->pointer()->focus()->parent())
            seat()->pointer()->focus()->topmostParent()->raise();
        else
            seat()->pointer()->focus()->raise();
    }
    // Left button released
    else
    {
        seat()->pointer()->stopResizingToplevel();
        seat()->pointer()->stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        seat()->pointer()->setDraggingSurface(nullptr);

        if (seat()->pointer()->focus()->imp()->lastPointerEventView)
        {
            if (!imp()->pointerIsOverView(seat()->pointer()->focus()->imp()->lastPointerEventView, cursor()->pos()))
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->setFocus(nullptr);
            }
        }
        else
        {
            if (!seat()->pointer()->focus()->inputRegion().containsPoint(cursor()->pos() - seat()->pointer()->focus()->pos()))
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                seat()->pointer()->setFocus(nullptr);
            }
        }
    }
}

void LScene::handlePointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, LPointer::AxisSource source)
{
    // Prevent recursive calls
    if (imp()->handlingPointerAxisEvent)
        return;

    imp()->listChanged = false;
    imp()->handlingPointerAxisEvent = true;
    LView::LViewPrivate::removeFlagWithChildren(mainView(), LVS::PointerAxisDone);
    imp()->handlePointerAxisEvent(mainView(), axisX, axisY, discreteX, discreteY, source);
    imp()->handlingPointerAxisEvent = false;

    if (!handleWaylandPointerEventsEnabled())
        return;

    seat()->pointer()->sendAxisEvent(axisX, axisY, discreteX, discreteY, source);
}

bool LScene::handleWaylandPointerEventsEnabled() const
{
    return imp()->handleWaylandPointerEvents;
}

void LScene::enableHandleWaylandPointerEvents(bool enabled)
{
    imp()->handleWaylandPointerEvents = enabled;
}

void LScene::handleKeyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    // Prevent recursive calls
    if (imp()->handlingKeyModifiersEvent)
        return;

    imp()->listChanged = false;
    imp()->handlingKeyModifiersEvent = true;
    LView::LViewPrivate::removeFlagWithChildren(mainView(), LVS::KeyModifiersDone);
    imp()->handleKeyModifiersEvent(mainView(), depressed, latched, locked, group);
    imp()->handlingKeyModifiersEvent = false;

    if (handleWaylandKeyboardEventsEnabled())
        seat()->keyboard()->sendModifiersEvent(depressed, latched, locked, group);
}

void LScene::handleKeyEvent(UInt32 keyCode, LKeyboard::KeyState keyState)
{
    // Prevent recursive calls
    if (imp()->handlingKeyEvent)
        return;

    imp()->listChanged = false;
    imp()->handlingKeyEvent = true;
    LView::LViewPrivate::removeFlagWithChildren(mainView(), LVS::KeyDone);
    imp()->handleKeyEvent(mainView(), keyCode, keyState);
    imp()->handlingKeyEvent = false;

    if (handleWaylandKeyboardEventsEnabled())
        seat()->keyboard()->sendKeyEvent(keyCode, keyState);

    if (!auxKeyboardImplementationEnabled())
        return;

    bool L_CTRL = seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL);
    bool L_SHIFT = seat()->keyboard()->isKeyCodePressed(KEY_LEFTSHIFT);
    bool mods = seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT) && L_CTRL;

    if (keyState == LKeyboard::Released)
    {
        // Terminates client connection
        if (L_CTRL && seat()->keyboard()->keySymbol(keyCode) == XKB_KEY_q)
        {
            if (seat()->keyboard()->focus())
                seat()->keyboard()->focus()->client()->destroy();
        }

        // Minimizes currently focused surface
        else if (L_CTRL && seat()->keyboard()->keySymbol(keyCode) == XKB_KEY_m)
        {
            if (seat()->keyboard()->focus() && seat()->keyboard()->focus()->toplevel() && !seat()->keyboard()->focus()->toplevel()->fullscreen())
                seat()->keyboard()->focus()->toplevel()->setMinimizedRequest();
        }

        // Terminates the compositor
        else if (keyCode == KEY_ESC && L_CTRL && L_SHIFT)
            compositor()->finish();

        // Screenshot
        else if (L_CTRL && L_SHIFT && keyCode == KEY_3)
        {
            if (cursor() && cursor()->output() && cursor()->output()->bufferTexture(0))
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

        else if (L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
    }

    // Key press
    else
    {
        // Launches weston-terminal
        if (keyCode == KEY_F1 && !mods)
            LLauncher::launch("weston-terminal");

        // CTRL sets Copy as the preferred action in drag & drop sesión
        if (L_CTRL)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);

        // SHIFT sets the Move as the preferred action in drag & drop sesión
        else if (L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
    }
}

bool LScene::handleWaylandKeyboardEventsEnabled() const
{
    return imp()->handleWaylandKeyboardEvents;
}

void LScene::enableHandleWaylandKeyboardEvents(bool enabled)
{
    imp()->handleWaylandKeyboardEvents = enabled;
}

bool LScene::auxKeyboardImplementationEnabled() const
{
    return imp()->auxKeyboardImplementationEnabled;
}

void LScene::enableAuxKeyboardImplementation(bool enabled)
{
    imp()->auxKeyboardImplementationEnabled = enabled;
}

LSceneView *LScene::mainView() const
{
    return &imp()->view;
}

LView *LScene::viewAt(const LPoint &pos)
{
    return imp()->viewAt(mainView(), pos);
}
