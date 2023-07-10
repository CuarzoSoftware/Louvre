#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>

void LScene::LScenePrivate::clearTmpVariables(OutputData &oD)
{
    oD.opaqueTransposedCSum.clear();
}

void LScene::LScenePrivate::damageAll(OutputData &oD)
{
    oD.prevDamageC.clear();
    oD.newDamageC.clear();
    oD.prevDamageC.addRect(oD.o->rectC());
    oD.newDamageC.addRect(oD.o->rectC());
}

void LScene::LScenePrivate::checkRectChange(OutputData &oD)
{
    if (oD.prevRectC != oD.o->rectC())
    {
        damageAll(oD);
        oD.prevRectC = oD.o->rectC();
    }
}

void LScene::LScenePrivate::calcNewDamage(LView *view, OutputData &oD, bool force, LRegion *opacity)
{
    // Calculate damage
    if (force)
    {
        // Quick output data handle
        view->imp()->currentOutputData = &view->imp()->outputsMap[oD.o];

        // Cache mapped call
        view->imp()->currentMapped = view->mapped();

        // If the scale is equal to the global scale, we avoid performing transformations later
        view->imp()->bufferScaleMatchGlobalScale = view->scale() == compositor()->globalScale();

        // Cache view pos
        view->imp()->currentRectC.setPos(view->posC());

        // Make view pos fit output pixels grid
        if (oD.o->scale() != compositor()->globalScale())
        {
            view->imp()->currentRectC.setX(view->imp()->currentRectC.x() - view->imp()->currentRectC.x() % compositor()->globalScale());
            view->imp()->currentRectC.setY(view->imp()->currentRectC.y() - view->imp()->currentRectC.y() % compositor()->globalScale());
        }

        view->imp()->cacheScalingEnabled = view->scalingEnabled() && view->sizeC() != view->scaledSizeC();

        // Cache view size
        if (view->imp()->cacheScalingEnabled)
        {
            view->imp()->currentRectC.setSize(view->scaledSizeC());
            view->imp()->axisScalig.setW(((Float32)view->scaledSizeC().w()/(Float32)view->sizeC().w()));
            view->imp()->axisScalig.setH(((Float32)view->scaledSizeC().h()/(Float32)view->sizeC().h()));
        }
        else
            view->imp()->currentRectC.setSize(view->sizeC());

        // We clear damage only
        bool clearDamage = true;

        // Update view intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(view->imp()->currentRectC, false))
                view->enterOutput(oD.o);
            else
               view->leaveOutput(oD.o);
        }

        if (!view->isRenderable())
            return;

        if (view->imp()->currentRectC.size().area() == 0)
            view->imp()->currentMapped = false;

        bool mappingChanged = view->imp()->currentMapped != view->imp()->currentOutputData->prevMapped;

        if (!mappingChanged && !view->imp()->currentMapped)
        {
            if (view->forceRequestNextFrameEnabled())
                view->requestNextFrame(oD.o);
            return;
        }

        view->imp()->cachedMultipliedOpacity = view->multipliedOpacity();

        bool multipliedOpacityChanged = view->imp()->cachedMultipliedOpacity != view->imp()->currentOutputData->prevMultipliedOpacity;
        bool rectChanged = view->imp()->currentRectC != view->imp()->currentOutputData->previousRectC;

        // If rect or order changed (set current rect and prev rect as damage)
        if (mappingChanged || rectChanged || view->imp()->currentOutputData->changedOrder || multipliedOpacityChanged)
        {
            view->imp()->currentDamageTransposedC.clear();

            if ((rectChanged && view->imp()->currentMapped) || (mappingChanged && !view->imp()->currentMapped) )
                view->imp()->currentDamageTransposedC.addRect(view->imp()->currentOutputData->previousRectC);

            if (view->imp()->currentMapped)
                view->imp()->currentDamageTransposedC.addRect(view->imp()->currentRectC);

            view->imp()->currentOutputData->changedOrder = false;
            view->imp()->currentOutputData->prevMapped = view->imp()->currentMapped;
            view->imp()->currentOutputData->previousRectC = view->imp()->currentRectC;
            view->imp()->currentOutputData->prevMultipliedOpacity = view->imp()->cachedMultipliedOpacity;
        }
        else
        {
            if (view->imp()->cacheScalingEnabled)
            {
                LRegion copy = *view->damageC();
                view->imp()->currentDamageTransposedC.clear();
                oD.boxes = copy.rects(&oD.n);

                for (Int32 i = 0; i < oD.n; i++)
                {
                    view->imp()->currentDamageTransposedC.addRect(
                        view->imp()->axisScalig.x()*(oD.boxes->x1) + view->imp()->currentRectC.pos().x(),
                        view->imp()->axisScalig.y()*(oD.boxes->y1) + view->imp()->currentRectC.pos().y(),
                        view->imp()->axisScalig.x()*(oD.boxes->x2 - oD.boxes->x1),
                        view->imp()->axisScalig.y()*(oD.boxes->y2 - oD.boxes->y1));

                    oD.boxes++;
                }
            }
            else
            {
                view->imp()->currentDamageTransposedC = *view->damageC();
                view->imp()->currentDamageTransposedC.offset(view->imp()->currentRectC.pos());
            }
        }

        // Check if there is parent clipping
        LRegion clipping;
        clipping.addRect(view->imp()->currentRectC);
        parentClipping(view->parent(), &clipping);

        if (!view->imp()->currentOutputData->prevParentClippingC.empty())
        {
            LRegion clipDiff = clipping;
            clipDiff.subtractRegion(view->imp()->currentOutputData->prevParentClippingC);
            view->imp()->currentOutputData->prevParentClippingC.subtractRegion(clipping);
            oD.newDamageC.addRegion(view->imp()->currentOutputData->prevParentClippingC);
            view->imp()->currentDamageTransposedC.addRegion(clipDiff);
        }

        view->imp()->currentOutputData->prevParentClippingC = clipping;

        view->imp()->currentDamageTransposedC.intersectRegion(view->imp()->currentOutputData->prevParentClippingC);

        // Remove previus opaque region to view damage
        view->imp()->currentDamageTransposedC.subtractRegion(oD.opaqueTransposedCSum);

        // Add clipped damage to new damage
        oD.newDamageC.addRegion(view->imp()->currentDamageTransposedC);

        if (opacity)
            view->imp()->currentDamageTransposedC.subtractRegion(*opacity);

        if (view->opacity() < 1.f)
        {
            view->imp()->currentTraslucentTransposedC.clear();
            view->imp()->currentTraslucentTransposedC.addRect(view->imp()->currentRectC);
            view->imp()->currentOpaqueTransposedC.clear();
        }
        else
        {
            if (view->imp()->cacheScalingEnabled)
            {
                LRegion copy = *view->translucentRegionC();
                view->imp()->currentTraslucentTransposedC.clear();
                oD.boxes = copy.rects(&oD.n);

                for (Int32 i = 0; i < oD.n; i++)
                {
                    view->imp()->currentTraslucentTransposedC.addRect(
                        view->imp()->axisScalig.x()*(oD.boxes->x1) + view->imp()->currentRectC.pos().x(),
                        view->imp()->axisScalig.y()*(oD.boxes->y1) + view->imp()->currentRectC.pos().y(),
                        view->imp()->axisScalig.x()*(oD.boxes->x2 - oD.boxes->x1),
                        view->imp()->axisScalig.y()*(oD.boxes->y2 - oD.boxes->y1));

                    oD.boxes++;
                }

                copy = *view->opaqueRegionC();
                view->imp()->currentOpaqueTransposedC.clear();
                oD.boxes = copy.rects(&oD.n);

                for (Int32 i = 0; i < oD.n; i++)
                {
                    view->imp()->currentOpaqueTransposedC.addRect(
                        view->imp()->axisScalig.x()*(oD.boxes->x1) + view->imp()->currentRectC.pos().x(),
                        view->imp()->axisScalig.y()*(oD.boxes->y1) + view->imp()->currentRectC.pos().y(),
                        view->imp()->axisScalig.x()*(oD.boxes->x2 - oD.boxes->x1),
                        view->imp()->axisScalig.y()*(oD.boxes->y2 - oD.boxes->y1));

                    oD.boxes++;
                }
            }
            else
            {
                // Store tansposed traslucent region
                view->imp()->currentTraslucentTransposedC = *view->translucentRegionC();
                view->imp()->currentTraslucentTransposedC.offset(view->imp()->currentRectC.pos());

                // Store tansposed opaque region
                view->imp()->currentOpaqueTransposedC = *view->opaqueRegionC();
                view->imp()->currentOpaqueTransposedC.offset(view->imp()->currentRectC.pos());
            }
        }

        view->imp()->currentOpaqueTransposedC.intersectRegion(view->imp()->currentOutputData->prevParentClippingC);
        view->imp()->currentTraslucentTransposedC.intersectRegion(view->imp()->currentOutputData->prevParentClippingC);

        // Check if view is ocludded
        LRegion ocluddedTest;
        ocluddedTest.addRect(view->imp()->currentRectC);
        ocluddedTest.subtractRegion(oD.opaqueTransposedCSum);
        view->imp()->occluded = ocluddedTest.empty();

        if ((!view->imp()->occluded && clearDamage) || view->forceRequestNextFrameEnabled())
            view->requestNextFrame(oD.o);

        if (opacity)
        {
            view->imp()->currentOpaqueTransposedCSum = *opacity;
            opacity->addRegion(view->imp()->currentOpaqueTransposedC);
        }
        else
        {
            // Store sum of previus opaque regions
            view->imp()->currentOpaqueTransposedCSum = oD.opaqueTransposedCSum;
            oD.opaqueTransposedCSum.addRegion(view->imp()->currentOpaqueTransposedC);
        }

        return;
    }

    if (view->opacity() < 1.f)
    {
        view->imp()->childrenOpaqueTransposedCSum.clear();
        for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
            calcNewDamage(*it, oD, false, &view->imp()->childrenOpaqueTransposedCSum);
    }
    else
    {
        for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
            calcNewDamage(*it, oD, false, opacity);
    }

    calcNewDamage(view, oD, true, opacity);
}

void LScene::LScenePrivate::drawOpaqueDamage(LView *view, OutputData &oD, bool force)
{
    if (force)
    {
        if (!view->isRenderable() || !view->imp()->currentMapped || view->imp()->occluded || view->imp()->cachedMultipliedOpacity < 1.f)
            return;

        view->imp()->currentOpaqueTransposedC.intersectRegion(oD.newDamageC);
        view->imp()->currentOpaqueTransposedC.subtractRegion(view->imp()->currentOpaqueTransposedCSum);

        oD.boxes = view->imp()->currentOpaqueTransposedC.rects(&oD.n);

        if (view->imp()->cacheScalingEnabled)
        {
            // Draw opaque rects
            for (Int32 i = 0; i < oD.n; i++)
            {
                oD.w = oD.boxes->x2 - oD.boxes->x1;
                oD.h = oD.boxes->y2 - oD.boxes->y1;

                view->paintRect(
                    oD.p,
                    (oD.boxes->x1 - view->imp()->currentRectC.x())/view->imp()->axisScalig.x(),
                    (oD.boxes->y1 - view->imp()->currentRectC.y())/view->imp()->axisScalig.y(),
                    oD.w/view->imp()->axisScalig.x(),
                    oD.h/view->imp()->axisScalig.y(),
                    oD.boxes->x1,
                    oD.boxes->y1,
                    oD.w,
                    oD.h,
                    view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                    1.f);

                oD.boxes++;
            }
        }
        else
        {
            // Draw opaque rects
            for (Int32 i = 0; i < oD.n; i++)
            {
                oD.w = oD.boxes->x2 - oD.boxes->x1;
                oD.h = oD.boxes->y2 - oD.boxes->y1;

                view->paintRect(
                    oD.p,
                    oD.boxes->x1 - view->imp()->currentRectC.x(),
                    oD.boxes->y1 - view->imp()->currentRectC.y(),
                    oD.w,
                    oD.h,
                    oD.boxes->x1,
                    oD.boxes->y1,
                    oD.w,
                    oD.h,
                    view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                    1.f);

                oD.boxes++;
            }
        }

        return;
    }

    for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
        drawOpaqueDamage(*it, oD);

    drawOpaqueDamage(view, oD, true);
}

void LScene::LScenePrivate::drawBackground(OutputData &oD)
{
    LRegion backgroundDamage = oD.newDamageC;
    backgroundDamage.subtractRegion(oD.opaqueTransposedCSum);
    oD.boxes = backgroundDamage.rects(&oD.n);

    for (Int32 i = 0; i < oD.n; i++)
    {
        oD.p->drawColorC(oD.boxes->x1,
                      oD.boxes->y1,
                      oD.boxes->x2 - oD.boxes->x1,
                      oD.boxes->y2 - oD.boxes->y1,
                      clearColor.r,
                      clearColor.g,
                      clearColor.b,
                      1.f);
        oD.boxes++;
    }
}

void LScene::LScenePrivate::drawTranslucentDamage(LView *view, OutputData &oD, bool force, LRegion *opacity)
{
    if (force)
    {
        if (!view->isRenderable() || !view->imp()->currentMapped || view->imp()->occluded)
            return;

        view->imp()->occluded = true;

        if (opacity)
        {
            view->imp()->currentTraslucentTransposedC.clear();
            view->imp()->currentTraslucentTransposedC.addRect(view->imp()->currentRectC);
            view->imp()->currentTraslucentTransposedC.intersectRegion(oD.newDamageC);
            view->imp()->currentTraslucentTransposedC.subtractRegion(view->imp()->currentOpaqueTransposedCSum);
        }
        else
        {
            view->imp()->currentTraslucentTransposedC.intersectRegion(oD.newDamageC);
            view->imp()->currentTraslucentTransposedC.subtractRegion(view->imp()->currentOpaqueTransposedCSum);
        }

        oD.boxes = view->imp()->currentTraslucentTransposedC.rects(&oD.n);

        // Draw transulcent rects
        if (view->imp()->cacheScalingEnabled)
        {
            for (Int32 i = 0; i < oD.n; i++)
            {
                oD.w = oD.boxes->x2 - oD.boxes->x1;
                oD.h = oD.boxes->y2 - oD.boxes->y1;

                view->paintRect(
                    oD.p,
                    (oD.boxes->x1 - view->imp()->currentRectC.x())/view->imp()->axisScalig.x(),
                    (oD.boxes->y1 - view->imp()->currentRectC.y())/view->imp()->axisScalig.y(),
                    oD.w/view->imp()->axisScalig.x(),
                    oD.h/view->imp()->axisScalig.y(),
                    oD.boxes->x1,
                    oD.boxes->y1,
                    oD.w,
                    oD.h,
                    view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                    view->imp()->cachedMultipliedOpacity);

                oD.boxes++;
            }
        }
        else
        {
            for (Int32 i = 0; i < oD.n; i++)
            {
                oD.w = oD.boxes->x2 - oD.boxes->x1;
                oD.h = oD.boxes->y2 - oD.boxes->y1;

                view->paintRect(
                    oD.p,
                    oD.boxes->x1 - view->imp()->currentRectC.x(),
                    oD.boxes->y1 - view->imp()->currentRectC.y(),
                    oD.w,
                    oD.h,
                    oD.boxes->x1 ,
                    oD.boxes->y1,
                    oD.w,
                    oD.h,
                    view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                    view->imp()->cachedMultipliedOpacity);

                oD.boxes++;
            }
        }
        return;
    }

    drawTranslucentDamage(view, oD, true, opacity);

    if (view->opacity() < 1.f)
    {
        for (LView *child : view->children())
            drawTranslucentDamage(child, oD, false, &view->imp()->childrenOpaqueTransposedCSum);
    }
    else
    {
        for (LView *child : view->children())
            drawTranslucentDamage(child, oD, false, opacity);
    }
}

LView *LScene::LScenePrivate::viewAtC(LView *view, const LPoint &pos)
{
    LView *v = nullptr;

    for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
    {
        v = viewAtC(*it, pos);

        if (v)
            return v;
    }

    if (!view->mapped() || !view->inputEnabled())
        return nullptr;

    if (view->imp()->cacheScalingEnabled)
    {
        if (!pointClippedByParent(view->parent(), pos) && view->inputRegionC()->containsPoint((pos - view->posC())/view->imp()->axisScalig))
            v = view;
    }
    else
    {
        if (!pointClippedByParent(view->parent(), pos) && view->inputRegionC()->containsPoint(pos - view->posC()))
            v = view;
    }

    return v;
}

bool LScene::LScenePrivate::pointClippedByParent(LView *parent, const LPoint &point)
{
    if (!parent)
        return false;

    if (parent->clippingEnabled())
    {
        if (!parent->imp()->currentRectC.containsPoint(point))
            return true;
    }

    return pointClippedByParent(parent->parent(), point);
}

void LScene::LScenePrivate::parentClipping(LView *parent, LRegion *region)
{
    if (!parent)
        return;

    if (parent->clippingEnabled())
        region->clip(LRect(parent->posC(), parent->sizeC()));

    parentClipping(parent->parent(), region);
}
