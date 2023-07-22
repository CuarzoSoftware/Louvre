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
            if (LRect(view->pos(), view->size()).containsPoint(pos))
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

bool LScene::LScenePrivate::pointerIsOverView(LView *view, const LPoint &pos)
{
    if (!view->mapped() || !view->inputEnabled())
        return false;

    if (pointClippedByParent(view, pos))
        return false;

    if (pointClippedByParentScene(view, pos))
        return false;

    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector() != LSizeF(1.f,1.f))
    {
        if (view->scalingVector().area() == 0.f)
            return false;

        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint((pos - view->pos())/view->scalingVector()))
                return true;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint((pos - view->pos())/view->scalingVector()))
                return true;
        }
    }
    else
    {
        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint(pos - view->pos()))
                return true;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint(pos))
                return true;
        }
    }

    return false;
}

void LScene::LScenePrivate::handlePointerMove(LView *view, const LPoint &pos, LView **firstViewFound)
{
    for (list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        handlePointerMove(*it, pos, firstViewFound);

    if (pointerIsOverView(view, pos))
    {
        if (!(*firstViewFound))
            *firstViewFound = view;

        if (view->pointerIsOver())
            view->pointerMoveEvent(viewLocalPos(view, pos));
        else
        {
            view->imp()->pointerIsOver = true;
            view->pointerEnterEvent(viewLocalPos(view, pos));
        }
    }
    else
    {
        if (view->pointerIsOver())
        {
            view->imp()->pointerIsOver = false;
            view->pointerLeaveEvent();
        }
    }

    // Hides unused warning
    (void)firstViewFound;
}


LPoint LScene::LScenePrivate::viewLocalPos(LView *view, const LPoint &pos)
{
    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector().area() != 0.f)
        return (pos - view->pos()) / view->scalingVector();
    else
        return pos - view->pos();
}

void LScene::LScenePrivate::handlePointerButton(LView *view, LPointer::Button button, LPointer::ButtonState state)
{
    for (list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        handlePointerButton(*it, button, state);

    if (view->imp()->pointerIsOver)
        view->pointerButtonEvent(button, state);
}

void LScene::LScenePrivate::handlePointerAxisEvent(LView *view, Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    for (list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        handlePointerAxisEvent(*it, axisX, axisY, discreteX, discreteY, source);

    if (view->imp()->pointerIsOver)
        view->pointerAxisEvent(axisX, axisY, discreteX, discreteY, source);
}
