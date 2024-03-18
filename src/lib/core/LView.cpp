#include <private/LCompositorPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneTouchPointPrivate.h>
#include <LTouchCancelEvent.h>
#include <LOutput.h>
#include <LLog.h>
#include <string.h>

using namespace Louvre;

LView::LView(UInt32 type, bool renderable, LView *parent) noexcept  : m_type(type)
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

    m_state.setFlag(KeyboardEvents, enabled);

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

    m_state.setFlag(TouchEvents, enabled);

    if (scene())
    {
        if (!enabled)
        {
            for (auto *tp : scene()->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->imp()->views.end();)
                {
                    if (*it == this)
                    {
                        it = tp->imp()->views.erase(it);
                        tp->imp()->listChanged = true;
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

    m_state.setFlag(PointerEvents, enabled);

    if (!enabled)
    {
        if (m_state.check(PointerIsOver))
        {
            if (scene())
            {
                LVectorRemoveOne(scene()->imp()->pointerFocus, this);
                scene()->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            }

            m_state.remove(PointerIsOver | PendingSwipeEnd | PendingPinchEnd | PendingHoldEnd);
        }
    }
}

void LView::repaint() const noexcept
{
    if (m_state.check(RepaintCalled))
        return;

    for (LOutput *o : outputs())
        o->repaint();

    m_state.add(RepaintCalled);
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
        damageScene(parentSceneView(), true);

        if (s != nullptr)
            sceneChanged(nullptr);
    }

    markAsChangedOrder();
    m_parent = view;
}

void LView::insertAfter(LView *prev) noexcept
{
    if (prev == this)
        return;

    if (prev)
    {
        setParent(prev->parent());
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

    // If prev == nullptr, insert to the front of current parent children list
    else
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

    auto sit { sceneView->m_sceneThreadsMap.find(thread) };

    if (sit != sceneView->m_sceneThreadsMap.end())
        sceneView->m_sceneThreadsMap.erase(sit);
}

void LView::markAsChangedOrder(bool includeChildren)
{
    for (auto &pair : m_threadsMap)
        pair.second.changedOrder = true;

    if (includeChildren)
        for (LView *child : children())
            child->markAsChangedOrder();
}

void LView::damageScene(LSceneView *scene, bool includeChildren)
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

        if (includeChildren)
            for (LView *child : children())
                child->damageScene(child->parentSceneView(), includeChildren);
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
            LVectorRemoveOne(scene()->imp()->pointerFocus, this);
            scene()->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            m_state.remove(PointerIsOver | PendingHoldEnd | PendingPinchEnd | PendingSwipeEnd);
        }

        if (m_state.check(TouchEvents))
        {
            for (auto *tp : scene()->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->views().end();)
                {
                    if (*it == this)
                    {
                        it = tp->imp()->views.erase(it);
                        tp->imp()->listChanged = true;
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

    for (LView *child : children())
        child->sceneChanged(newScene);
}
