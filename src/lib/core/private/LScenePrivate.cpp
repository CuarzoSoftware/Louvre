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

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
    {
        v = viewAt(*it, pos);

        if (v)
            return v;
    }

    if (!view->mapped() || !view->inputEnabled())
        return nullptr;

    if (view->clippingEnabled() && !view->clippingRect().containsPoint(pos))
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

    if (view->clippingEnabled() && !view->clippingRect().containsPoint(pos))
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

bool LScene::LScenePrivate::handlePointerMove(LView *view, const LPoint &pos, LView **firstViewFound)
{
    if (listChanged)
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handlePointerMove(*it, pos, firstViewFound))
            return false;

    if (!pointerIsBlocked && pointerIsOverView(view, pos))
    {
        if (!(*firstViewFound))
            *firstViewFound = view;

        if (pointerMoveSerial != view->imp()->pointerMoveSerial)
        {
            view->imp()->pointerMoveSerial = pointerMoveSerial;

            if (view->pointerIsOver())
            {
                view->pointerMoveEvent(viewLocalPos(view, pos));

                if (listChanged)
                    goto listChangedErr;
            }
            else
            {
                view->imp()->pointerIsOver = true;
                view->pointerEnterEvent(viewLocalPos(view, pos));

                if (listChanged)
                    goto listChangedErr;
            }
        }

        if (view->blockPointerEnabled())
            pointerIsBlocked = true;
    }
    else
    {
        if (pointerMoveSerial != view->imp()->pointerMoveSerial)
        {
            view->imp()->pointerMoveSerial = pointerMoveSerial;

            if (view->pointerIsOver())
            {
                view->imp()->pointerIsOver = false;
                view->pointerLeaveEvent();

                if (listChanged)
                    goto listChangedErr;
            }
        }
    }

    // Hides unused warning
    (void)firstViewFound;

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
    listChangedErr:
    listChanged = false;
    handlePointerMove(this->view, pos, nullptr);
    return false;
}


LPoint LScene::LScenePrivate::viewLocalPos(LView *view, const LPoint &pos)
{
    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector().area() != 0.f)
        return (pos - view->pos()) / view->scalingVector();
    else
        return pos - view->pos();
}

bool LScene::LScenePrivate::handlePointerButton(LView *view, LPointer::Button button, LPointer::ButtonState state)
{
    if (listChanged)
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handlePointerButton(*it, button, state))
            return false;

    if (view->imp()->pointerButtonSerial == pointerButtonSerial)
        return true;

    view->imp()->pointerButtonSerial = pointerButtonSerial;

    if (view->imp()->pointerIsOver)
        view->pointerButtonEvent(button, state);

    if (listChanged)
        goto listChangedErr;

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
    listChangedErr:
    listChanged = false;
    handlePointerButton(this->view, button, state);
    return false;
}

bool LScene::LScenePrivate::handlePointerAxisEvent(LView *view, Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    if (listChanged)
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handlePointerAxisEvent(*it, axisX, axisY, discreteX, discreteY, source))
            return false;

    if (view->imp()->pointerAxisSerial == pointerAxisSerial)
        return true;

    view->imp()->pointerAxisSerial = pointerAxisSerial;

    if (view->imp()->pointerIsOver)
        view->pointerAxisEvent(axisX, axisY, discreteX, discreteY, source);

    if (listChanged)
        goto listChangedErr;

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
    listChangedErr:
    listChanged = false;
    handlePointerAxisEvent(this->view, axisX, axisY, discreteX, discreteY, source);
    return false;
}

bool LScene::LScenePrivate::handleKeyModifiersEvent(LView *view, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    if (listChanged)
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handleKeyModifiersEvent(*it, depressed, latched, locked, group))
            return false;

    if (view->imp()->keyModifiersSerial == keyModifiersSerial)
        return true;

    view->imp()->keyModifiersSerial = keyModifiersSerial;

    view->keyModifiersEvent(depressed, latched, locked, group);

    if (listChanged)
        goto listChangedErr;

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
    listChangedErr:
    listChanged = false;
    handleKeyModifiersEvent(this->view, depressed, latched, locked, group);
    return false;
}

bool LScene::LScenePrivate::handleKeyEvent(LView *view, UInt32 keyCode, UInt32 keyState)
{
    if (listChanged)
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handleKeyEvent(*it, keyCode, keyState))
            return false;

    if (view->imp()->keySerial == keySerial)
        return true;

    view->imp()->keySerial = keySerial;

    view->keyEvent(keyCode, keyState);

    if (listChanged)
        goto listChangedErr;

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
    listChangedErr:
    listChanged = false;
    handleKeyEvent(this->view, keyCode, keyState);
    return false;
}
