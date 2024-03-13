#include <private/LCompositorPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSceneTouchPointPrivate.h>
#include <LTouchCancelEvent.h>
#include <LOutput.h>
#include <LLog.h>
#include <string.h>

using namespace Louvre;

using LVS = LView::LViewState;

LView::LView(UInt32 type, bool renderable, LView *parent) noexcept : m_type(type)
{
    m_state.setFlag(IsRenderable, renderable);
    compositor()->imp()->views.push_back(this);
    setParent(parent);
}

LView::~LView() noexcept
{
    setParent(nullptr);

    while (!children().empty())
        children().front()->setParent(nullptr);

    LVectorRemoveOneUnordered(compositor()->imp()->views, this);
}

void LView::enableKeyboardEvents(bool enabled) noexcept
{
    if (enabled == keyboardEventsEnabled())
        return;

    m_state.setFlag(LVS::KeyboardEvents, enabled);

    if (scene())
    {
        if (enabled)
            scene()->imp()->keyboardFocus.push_back(this);
        else
            LVectorRemoveOne(scene()->imp()->keyboardFocus, this);

        scene()->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
    }
}

void LView::enableTouchEvents(bool enabled) noexcept
{
    if (enabled == touchEventsEnabled())
        return;

    m_state.setFlag(LVS::TouchEvents, enabled);

    if (scene())
    {
        if (!enabled)
        {
            // TODO: unsafe ?
            for (auto *tp : scene()->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->imp()->views.end();)
                {
                    if ((*it) == this)
                    {
                        it = tp->imp()->views.erase(it);
                        tp->imp()->listChanged = true;
                        touchCancelEvent(LTouchCancelEvent());
                    }
                    else
                        it++;
                }
            }
        }
    }
}

LSceneTouchPoint *LView::findTouchPoint(Int32 id) const noexcept
{
    if (scene())
        for (auto *tp : scene()->touchPoints())
            if (tp->id() == id)
                return tp;

    return nullptr;
}

void LView::enablePointerEvents(bool enabled) noexcept
{
    if (enabled == pointerEventsEnabled())
        return;

    m_state.setFlag(LVS::PointerEvents, enabled);

    if (!enabled)
    {
        if (m_state.check(LVS::PointerIsOver))
        {
            if (scene())
            {
                if (m_state.check(LVS::PendingSwipeEnd))
                {
                    m_state.remove(LVS::PendingSwipeEnd);
                    scene()->imp()->pointerSwipeEndEvent.setCancelled(true);
                    scene()->imp()->pointerSwipeEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerSwipeEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                    pointerSwipeEndEvent(scene()->imp()->pointerSwipeEndEvent);
                }

                if (m_state.check(LVS::PendingPinchEnd))
                {
                    m_state.check(LVS::PendingPinchEnd);
                    scene()->imp()->pointerPinchEndEvent.setCancelled(true);
                    scene()->imp()->pointerPinchEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerPinchEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerPinchEndEvent.setSerial(LTime::nextSerial());
                    pointerPinchEndEvent(scene()->imp()->pointerPinchEndEvent);
                }

                if (m_state.check(LVS::PendingHoldEnd))
                {
                    m_state.remove(LVS::PendingHoldEnd);
                    scene()->imp()->pointerHoldEndEvent.setCancelled(true);
                    scene()->imp()->pointerHoldEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerHoldEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerHoldEndEvent.setSerial(LTime::nextSerial());
                    pointerHoldEndEvent(scene()->imp()->pointerHoldEndEvent);
                }

                LVectorRemoveOne(scene()->imp()->pointerFocus, this);
                scene()->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            }

            m_state.remove(LVS::PointerIsOver);
        }
    }
}

void LView::repaint() const noexcept
{
    if (m_state.check(LVS::RepaintCalled))
        return;

    for (LOutput *o : outputs())
        o->repaint();

    m_state.add(LVS::RepaintCalled);
}

void LView::setParent(LView *view) noexcept
{
    if (parent() == view || view == this)
        return;

    LScene *s { scene() };

    if (s)
        s->imp()->state.add(LScene::LScenePrivate::ChildrenListChanged);

    if (parent())
        parent()->m_children.erase(m_parentLink);

    if (view)
    {
        view->m_children.push_back(this);
        m_parentLink = std::prev(view->m_children.end());

        if (view->scene() != s)
            sceneChanged(view->scene());
    }
    else
    {
        damageScene(parentSceneView());

        if (s != nullptr)
            sceneChanged(nullptr);
    }

    markAsChangedOrder();
    m_parent = view;
}

void LView::insertAfter(LView *prev, bool switchParent) noexcept
{
    if (prev == this)
        return;

    // If prev == nullptr, insert to the front of current parent children list
    if (!prev)
    {
        // If no parent, is a no-op
        if (!parent())
            return;

        // Already in front
        if (parent()->children().front() == this)
            return;

        parent()->m_children.erase(m_parentLink);
        parent()->m_children.push_front(this);
        m_parentLink = parent()->m_children.begin();
        markAsChangedOrder();
        repaint();
    }
    else
    {
        if (switchParent)
            setParent(prev->parent());
        else if (prev->parent() != parent())
            return;

        markAsChangedOrder();
        repaint();

        if (!parent())
            return;

        if (prev == parent()->children().back())
        {
            parent()->m_children.erase(m_parentLink);
            parent()->m_children.push_back(this);
            m_parentLink = std::prev(parent()->m_children.end());
        }
        else
        {
            parent()->m_children.erase(m_parentLink);
            m_parentLink = parent()->m_children.insert(std::next(prev->m_parentLink), this);
        }
    }
}



void LView::removeThread(std::thread::id thread)
{
    auto it { m_threadsMap.find(thread) };

    if (it != m_threadsMap.end())
    {
        if (it->second.o)
            leftOutput(it->second.o);
        m_threadsMap.erase(it);
    }

    if (type() != Scene)
        return;

    LSceneView *sceneView { static_cast<LSceneView*>(this) };

    auto sit { sceneView->imp()->threadsMap.find(thread) };

    if (sit != sceneView->imp()->threadsMap.end())
        sceneView->imp()->threadsMap.erase(sit);
}

void LView::markAsChangedOrder(bool includeChildren)
{
    for (auto &pair : m_threadsMap)
        pair.second.changedOrder = true;

    if (includeChildren)
        for (LView *child : children())
            child->markAsChangedOrder();
}

void LView::damageScene(LSceneView *scene)
{
    if (scene)
    {
        for (auto &pair : m_threadsMap)
        {
            if (!pair.second.prevMapped)
                continue;

            if (pair.second.o)
                scene->addDamage(pair.second.o, pair.second.prevClipping);
        }

        for (LView *child : children())
            child->damageScene(child->parentSceneView());
    }
}

void LView::sceneChanged(LScene *newScene)
{
    if (scene())
    {
        if (m_state.check(KeyboardEvents))
        {
            LVectorRemoveOneUnordered(scene()->imp()->keyboardFocus, this);
            scene()->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
        }

        if (m_state.check(PointerIsOver))
        {
            if (m_state.check(PendingSwipeEnd))
            {
                m_state.remove(PendingSwipeEnd);
                scene()->imp()->pointerSwipeEndEvent.setCancelled(true);
                scene()->imp()->pointerSwipeEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                scene()->imp()->pointerSwipeEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                scene()->imp()->pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                pointerSwipeEndEvent(scene()->imp()->pointerSwipeEndEvent);
            }

            if (m_state.check(PendingPinchEnd))
            {
                m_state.remove(PendingPinchEnd);
                scene()->imp()->pointerPinchEndEvent.setCancelled(true);
                scene()->imp()->pointerPinchEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                scene()->imp()->pointerPinchEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                scene()->imp()->pointerPinchEndEvent.setSerial(LTime::nextSerial());
                pointerPinchEndEvent(scene()->imp()->pointerPinchEndEvent);
            }

            if (m_state.check(PendingHoldEnd))
            {
                m_state.remove(PendingHoldEnd);
                scene()->imp()->pointerHoldEndEvent.setCancelled(true);
                scene()->imp()->pointerHoldEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                scene()->imp()->pointerHoldEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                scene()->imp()->pointerHoldEndEvent.setSerial(LTime::nextSerial());
                pointerHoldEndEvent(scene()->imp()->pointerHoldEndEvent);
            }

            LVectorRemoveOne(scene()->imp()->pointerFocus, this);
            scene()->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            m_state.remove(PointerIsOver);
        }

        if (m_state.check(TouchEvents))
        {
            for (auto *tp : scene()->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->views().end();)
                {
                    if ((*it) == this)
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
        if (m_state.check(KeyboardEvents))
        {
            newScene->imp()->keyboardFocus.push_back(this);
            scene()->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
        }
    }

    m_scene = newScene;

    // TODO: UNSAFE
    for (LView *child : children())
        child->sceneChanged(newScene);
}
