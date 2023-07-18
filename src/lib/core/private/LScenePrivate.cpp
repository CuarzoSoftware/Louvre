#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>
#include <LLog.h>

LView *LScene::LScenePrivate::viewAtC(LView *view, const LPoint &pos)
{
    LView *v = nullptr;

    for (list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
    {
        v = viewAtC(*it, pos);

        if (v)
            return v;
    }

    if (!view->mapped() || !view->inputEnabled())
        return nullptr;

    if (pointClippedByParent(view, pos))
        return nullptr;

    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector() != LSizeF(1.f,1.f))
    {
        if (view->scalingVector().area() == 0.f)
            return nullptr;

        if (view->inputRegionC())
        {
            if (view->inputRegionC()->containsPoint((pos - view->posC())/view->scalingVector()))
                v = view;
        }
        else
        {
            if (LRect(view->posC(), view->sizeC()).containsPoint((pos - view->posC())/view->scalingVector()))
                v = view;
        }
    }
    else
    {
        if (view->inputRegionC())
        {
            if (view->inputRegionC()->containsPoint(pos - view->posC()))
                v = view;
        }
        else
        {
            if (LRect(view->posC(), view->sizeC()).containsPoint(pos - view->posC()))
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
        if (!LRect(view->parent()->posC(), view->parent()->sizeC()).containsPoint(point))
            return true;
    }

    return pointClippedByParent(view->parent(), point);
}
