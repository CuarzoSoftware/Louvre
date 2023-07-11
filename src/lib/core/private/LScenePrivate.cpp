#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>
#include <LLog.h>

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

void LScene::LScenePrivate::calcNewDamage(LView *view, OutputData &oD, bool force)
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
        if (!oD.allOutputsMatchGlobalScale)
        {
            view->imp()->currentRectC.setX(view->imp()->currentRectC.x() - (view->imp()->currentRectC.x() % compositor()->globalScale()));
            view->imp()->currentRectC.setY(view->imp()->currentRectC.y() - (view->imp()->currentRectC.y() % compositor()->globalScale()));
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

        // Update view intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(view->imp()->currentRectC, false))
                view->enterOutput(o);
            else
               view->leaveOutput(o);
        }

        if (!view->isRenderable())
            return;

        view->imp()->currentDamageTransposedC.clear();
        view->imp()->currentOpaqueTransposedC.clear();
        view->imp()->currentTraslucentTransposedC.clear();

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
                LLog::fatal("Scaling");
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

        // Calculates the current rect intersected with parents rects (when clipping enabled)
        LRegion currentParentClipping;
        currentParentClipping.addRect(view->imp()->currentRectC);
        parentClipping(view->parent(), &currentParentClipping);

        // Calculates the new exposed view region if parent clipping has grown
        LRegion newExposedParentClipping = currentParentClipping;
        newExposedParentClipping.subtractRegion(view->imp()->currentOutputData->prevParentClippingC);
        view->imp()->currentDamageTransposedC.addRegion(newExposedParentClipping);

        // Add exposed now non clipped region to new output damage
        view->imp()->currentOutputData->prevParentClippingC.subtractRegion(currentParentClipping);
        oD.newDamageC.addRegion(view->imp()->currentOutputData->prevParentClippingC);

        // Saves current clipped region for next frame
        view->imp()->currentOutputData->prevParentClippingC = currentParentClipping;

        // Clip current damage to current visible region
        view->imp()->currentDamageTransposedC.intersectRegion(currentParentClipping);

        // Remove previus opaque region to view damage
        view->imp()->currentDamageTransposedC.subtractRegion(oD.opaqueTransposedCSum);

        // Add clipped damage to new damage
        oD.newDamageC.addRegion(view->imp()->currentDamageTransposedC);

        if (view->imp()->cachedMultipliedOpacity < 1.f)
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

        // Clip opaque and translucent regions to current visible region
        view->imp()->currentOpaqueTransposedC.intersectRegion(currentParentClipping);
        view->imp()->currentTraslucentTransposedC.intersectRegion(currentParentClipping);

        // Check if view is ocludded
        currentParentClipping.subtractRegion(oD.opaqueTransposedCSum);

        view->imp()->occluded = currentParentClipping.empty();

        if (!view->imp()->occluded || view->forceRequestNextFrameEnabled())
            view->requestNextFrame(oD.o);

        // Store sum of previus opaque regions (this will later be clipped when painting opaque and translucent regions)
        view->imp()->currentOpaqueTransposedCSum = oD.opaqueTransposedCSum;
        oD.opaqueTransposedCSum.addRegion(view->imp()->currentOpaqueTransposedC);

        return;
    }


    for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
        calcNewDamage(*it, oD, false);

    calcNewDamage(view, oD, true);
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
            if (oD.outputMatchGlobalScale)
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
                Int32 x, y;
                for (Int32 i = 0; i < oD.n; i++)
                {
                    x = oD.boxes->x1 - (oD.boxes->x1 % compositor()->globalScale());
                    y = oD.boxes->y1 - (oD.boxes->y1 % compositor()->globalScale());

                    oD.w = oD.boxes->x2 - x;
                    oD.h = oD.boxes->y2 - y;

                    oD.w += oD.w % compositor()->globalScale();
                    oD.h += oD.h % compositor()->globalScale();

                    view->paintRect(
                        oD.p,
                        x - view->imp()->currentRectC.x(),
                        y - view->imp()->currentRectC.y(),
                        oD.w,
                        oD.h,
                        x,
                        y,
                        oD.w,
                        oD.h,
                        view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                        1.f);

                    oD.boxes++;
                }
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

void LScene::LScenePrivate::drawTranslucentDamage(LView *view, OutputData &oD, bool force)
{
    if (force)
    {
        if (!view->isRenderable() || !view->imp()->currentMapped || view->imp()->occluded)
            return;

        view->imp()->occluded = true;
        view->imp()->currentTraslucentTransposedC.intersectRegion(oD.newDamageC);
        view->imp()->currentTraslucentTransposedC.subtractRegion(view->imp()->currentOpaqueTransposedCSum);
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
            if (oD.outputMatchGlobalScale)
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
                Int32 x, y;
                for (Int32 i = 0; i < oD.n; i++)
                {
                    x = oD.boxes->x1 - (oD.boxes->x1 % compositor()->globalScale());
                    y = oD.boxes->y1 - (oD.boxes->y1 % compositor()->globalScale());

                    oD.w = oD.boxes->x2 - x;
                    oD.h = oD.boxes->y2 - y;

                    oD.w += oD.w % compositor()->globalScale();
                    oD.h += oD.h % compositor()->globalScale();

                    view->paintRect(
                        oD.p,
                        x - view->imp()->currentRectC.x(),
                        y - view->imp()->currentRectC.y(),
                        oD.w,
                        oD.h,
                        x,
                        y,
                        oD.w,
                        oD.h,
                        view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale(),
                        view->imp()->cachedMultipliedOpacity);

                    oD.boxes++;
                }
            }
        }
        return;
    }

    drawTranslucentDamage(view, oD, true);

    for (LView *child : view->children())
        drawTranslucentDamage(child, oD, false);

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

void LScene::LScenePrivate::checkOutputsScale(OutputData &oD)
{
    oD.outputMatchGlobalScale = oD.o->scale() == compositor()->globalScale();
    oD.allOutputsMatchGlobalScale = true;

    for (LOutput *o : compositor()->outputs())
    {
        if (o->scale() != compositor()->globalScale())
        {
            oD.allOutputsMatchGlobalScale = false;
            return;
        }
    }
}
