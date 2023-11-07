#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>

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

    LSceneView *sceneView = (LSceneView*)view;

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
