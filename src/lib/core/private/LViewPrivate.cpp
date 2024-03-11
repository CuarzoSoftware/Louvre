#include <LTouchCancelEvent.h>
#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSceneTouchPointPrivate.h>

void LView::LViewPrivate::removeThread(LView *view, std::thread::id thread)
{
    auto it = threadsMap.find(thread);

    if (it != threadsMap.end())
    {
        if (it->second.o)
            view->leftOutput(it->second.o);
        threadsMap.erase(it);
    }

    if (view->type() != Scene)
        return;

    LSceneView *sceneView { static_cast<LSceneView*>(view) };

    auto sit = sceneView->imp()->threadsMap.find(thread);

    if (sit != sceneView->imp()->threadsMap.end())
        sceneView->imp()->threadsMap.erase(sit);
}

void LView::LViewPrivate::markAsChangedOrder(bool includeChildren)
{
    for (auto &pair : threadsMap)
        pair.second.changedOrder = true;

    if (includeChildren)
        for (LView *child : children)
            child->imp()->markAsChangedOrder();
}

void LView::LViewPrivate::damageScene(LSceneView *s)
{
    if (s)
    {
        for (auto &pair : threadsMap)
        {
            if (!pair.second.prevMapped)
                continue;

            if (pair.second.o)
                s->addDamage(pair.second.o, pair.second.prevClipping);
        }

        for (LView *child : children)
            child->imp()->damageScene(child->parentSceneView());
    }
}

void LView::LViewPrivate::sceneChanged(LScene *newScene)
{
    if (currentScene)
    {
        if (hasFlag(KeyboardEvents))
        {
            LVectorRemoveOneUnordered(currentScene->imp()->keyboardFocus, view);
            currentScene->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
        }

        if (hasFlag(PointerIsOver))
        {
            if (hasFlag(PendingSwipeEnd))
            {
                removeFlag(PendingSwipeEnd);
                currentScene->imp()->pointerSwipeEndEvent.setCancelled(true);
                currentScene->imp()->pointerSwipeEndEvent.setMs(currentScene->imp()->currentPointerMoveEvent.ms());
                currentScene->imp()->pointerSwipeEndEvent.setUs(currentScene->imp()->currentPointerMoveEvent.us());
                currentScene->imp()->pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                view->pointerSwipeEndEvent(currentScene->imp()->pointerSwipeEndEvent);
            }

            if (hasFlag(PendingPinchEnd))
            {
                removeFlag(PendingPinchEnd);
                currentScene->imp()->pointerPinchEndEvent.setCancelled(true);
                currentScene->imp()->pointerPinchEndEvent.setMs(currentScene->imp()->currentPointerMoveEvent.ms());
                currentScene->imp()->pointerPinchEndEvent.setUs(currentScene->imp()->currentPointerMoveEvent.us());
                currentScene->imp()->pointerPinchEndEvent.setSerial(LTime::nextSerial());
                view->pointerPinchEndEvent(currentScene->imp()->pointerPinchEndEvent);
            }

            if (hasFlag(PendingHoldEnd))
            {
                removeFlag(PendingHoldEnd);
                currentScene->imp()->pointerHoldEndEvent.setCancelled(true);
                currentScene->imp()->pointerHoldEndEvent.setMs(currentScene->imp()->currentPointerMoveEvent.ms());
                currentScene->imp()->pointerHoldEndEvent.setUs(currentScene->imp()->currentPointerMoveEvent.us());
                currentScene->imp()->pointerHoldEndEvent.setSerial(LTime::nextSerial());
                view->pointerHoldEndEvent(currentScene->imp()->pointerHoldEndEvent);
            }

            LVectorRemoveOne(currentScene->imp()->pointerFocus, view);
            currentScene->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            removeFlag(PointerIsOver);
        }

        if (hasFlag(TouchEvents))
        {
            for (auto *tp : currentScene->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->views().end();)
                {
                    if ((*it) == view)
                    {
                        LView *v = *it;
                        it = tp->imp()->views.erase(it);
                        tp->imp()->listChanged = true;
                        v->touchCancelEvent(LTouchCancelEvent());
                    }
                    else
                        it++;
                }
            }
        }
    }

    if (newScene)
    {
        if (hasFlag(KeyboardEvents))
        {
            newScene->imp()->keyboardFocus.push_back(view);
            currentScene->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
        }
    }

    currentScene = newScene;

    // TODO: UNSAFE
    for (LView *child : children)
        child->imp()->sceneChanged(newScene);
}
