#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LSceneViewPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>
#include <LFramebuffer.h>
#include <LLog.h>

LView *LScene::LScenePrivate::viewAt(LView *view, const LPoint &pos)
{
    LView *v = nullptr;

    for (list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
    {
        v = viewAt(*it, pos);

        if (v)
            return v;
    }

    if (!view->mapped() || !view->inputEnabled())
        return nullptr;

    if (pointClippedByParent(view, pos))
        return nullptr;

    if (pointClippedByParentScene(view, pos))
        return nullptr;

    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector() != LSizeF(1.f,1.f))
    {
        if (view->scalingVector().area() == 0.f)
            return nullptr;

        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint((pos - view->pos())/view->scalingVector()))
                v = view;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint((pos - view->pos())/view->scalingVector()))
                v = view;
        }
    }
    else
    {
        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint(pos - view->pos()))
                v = view;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint(pos - view->pos()))
                v = view;
        }
    }

    return v;
}

bool LScene::LScenePrivate::pointClippedByParent(LView *view, const LPoint &point)
{
    if (!view->parent())
        return false;

    if (view->parentClippingEnabled())
    {
        if (!LRect(view->parent()->pos(), view->parent()->size()).containsPoint(point))
            return true;
    }

    return pointClippedByParent(view->parent(), point);
}

bool LScene::LScenePrivate::pointClippedByParentScene(LView *view, const LPoint &point)
{
    LSceneView *parentScene = view->parentSceneView();

    if (!parentScene)
        return false;

    if (parentScene->isLScene())
        return false;

    if (!parentScene->imp()->fb->rect().containsPoint(point))
        return true;

    return pointClippedByParentScene(parentScene, point);
}
