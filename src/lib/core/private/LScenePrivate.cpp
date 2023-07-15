#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>
#include <LLog.h>
#include <LPainterMask.h>

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
        // Quick view cache handle to reduce verbosity
        LView::LViewPrivate::ViewCache &cache = view->imp()->cache;

        // Quick output data handle
        cache.voD = &view->imp()->outputsMap[oD.o];

        // Cache mapped call
        cache.mapped = view->mapped();

        // If the scale is equal to the global scale, we avoid performing transformations later
        cache.bufferScaleMatchGlobalScale = view->bufferScale() == compositor()->globalScale();

        // Cache view rect
        cache.rect.setPos(view->posC());
        cache.rect.setSize(view->sizeC());

        cache.scalingVector = view->scalingVector();
        cache.scalingEnabled = (view->scalingEnabled() || view->parentScalingEnabled()) && cache.scalingVector != LSizeF(1.f, 1.f);

        // Make view pos fit output pixels grid
        if (!oD.allOutputsMatchGlobalScale || cache.scalingEnabled)
        {
            cache.rect.setX(cache.rect.x() - (cache.rect.x() % compositor()->globalScale()));
            cache.rect.setY(cache.rect.y() - (cache.rect.y() % compositor()->globalScale()));
            cache.rect.setW(cache.rect.w() + (cache.rect.w() % compositor()->globalScale()));
            cache.rect.setH(cache.rect.h() + (cache.rect.h() % compositor()->globalScale()));
        }

        // Update view intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(cache.rect, false))
                view->enteredOutput(o);
            else
               view->leftOutput(o);
        }

        if (!view->isRenderable())
            return;

        cache.opacity = view->opacity();

        if (cache.rect.size().area() == 0 || cache.opacity <= 0.f || cache.scalingVector.w() == 0.f || cache.scalingVector.y() == 0.f)
            cache.mapped = false;

        bool mappingChanged = cache.mapped != cache.voD->prevMapped;

        if (!mappingChanged && !cache.mapped)
        {
            if (view->forceRequestNextFrameEnabled())
                view->requestNextFrame(oD.o);
            return;
        }

        bool opacityChanged = cache.opacity != cache.voD->prevOpacity;

        bool rectChanged = cache.rect != cache.voD->prevRect;

        // If rect or order changed (set current rect and prev rect as damage)
        if (mappingChanged || rectChanged || cache.voD->changedOrder || opacityChanged || cache.scalingEnabled || !view->damageC())
        {
            cache.damage.addRect(cache.rect);
            cache.voD->changedOrder = false;
            cache.voD->prevMapped = cache.mapped;
            cache.voD->prevRect = cache.rect;
            cache.voD->prevOpacity = cache.opacity;

            if (!cache.mapped)
            {
                oD.newDamageC.addRegion(cache.voD->prevParentClipping);
                return;
            }
        }
        else
        {
            cache.damage = *view->damageC();
            cache.damage.offset(cache.rect.pos());
        }

        // Calculates the current rect intersected with parents rects (when clipping enabled)
        LRegion currentParentClipping;
        currentParentClipping.addRect(cache.rect);

        if (view->parentClippingEnabled())
            parentClipping(view->parent(), &currentParentClipping);

        // Calculates the new exposed view region if parent clipping has grown
        LRegion newExposedParentClipping = currentParentClipping;
        newExposedParentClipping.subtractRegion(cache.voD->prevParentClipping);
        cache.damage.addRegion(newExposedParentClipping);

        // Add exposed now non clipped region to new output damage
        cache.voD->prevParentClipping.subtractRegion(currentParentClipping);
        oD.newDamageC.addRegion(cache.voD->prevParentClipping);

        // Saves current clipped region for next frame
        cache.voD->prevParentClipping = currentParentClipping;

        // Clip current damage to current visible region
        cache.damage.intersectRegion(currentParentClipping);

        // Remove previus opaque region to view damage
        cache.damage.subtractRegion(oD.opaqueTransposedCSum);

        // Add clipped damage to new damage
        oD.newDamageC.addRegion(cache.damage);

        if (cache.opacity < 1.f || cache.scalingEnabled)
        {
            cache.translucent.clear();
            cache.translucent.addRect(cache.rect);
            cache.opaque.clear();
        }
        else
        {
            // Store tansposed traslucent region
            if (view->translucentRegionC())
            {
                cache.translucent = *view->translucentRegionC();
                cache.translucent.offset(cache.rect.pos());
            }
            else
            {
                cache.translucent.clear();
                cache.translucent.addRect(cache.rect);
            }

            // Store tansposed opaque region
            if (view->opaqueRegionC())
            {
                cache.opaque = *view->opaqueRegionC();
                cache.opaque.offset(cache.rect.pos());
            }
            else
            {
                cache.opaque = cache.translucent;
                cache.opaque.inverse(cache.rect);
            }
        }

        LRegion masksRegion;
        LPainterMask *mask;
        for (UInt32 i = 0; i < 7; i++)
            if ((mask = view->getMask(i)))
                masksRegion.addRect(mask->rectC());
        masksRegion.offset(cache.rect.pos());
        masksRegion.clip(cache.rect);

        // Apply masks region
        cache.opaque.subtractRegion(masksRegion);
        cache.translucent.addRegion(masksRegion);

        // Clip opaque and translucent regions to current visible region
        cache.opaque.intersectRegion(currentParentClipping);
        cache.translucent.intersectRegion(currentParentClipping);

        // Check if view is ocludded
        currentParentClipping.subtractRegion(oD.opaqueTransposedCSum);

        cache.occluded = currentParentClipping.empty();

        if (!cache.occluded || view->forceRequestNextFrameEnabled())
            view->requestNextFrame(oD.o);

        // Store sum of previus opaque regions (this will later be clipped when painting opaque and translucent regions)
        cache.opaqueOverlay = oD.opaqueTransposedCSum;
        oD.opaqueTransposedCSum.addRegion(cache.opaque);
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
        LView::LViewPrivate::ViewCache &cache = view->imp()->cache;

        if (!view->isRenderable() || !cache.mapped || cache.occluded || cache.opacity < 1.f)
            return;

        cache.opaque.intersectRegion(oD.newDamageC);
        cache.opaque.subtractRegion(cache.opaqueOverlay);

        oD.boxes = cache.opaque.rects(&oD.n);

        if (cache.scalingEnabled)
        {
            Int32 sX,sY,sW,sH,dX,dY,dW,dH;

            for (Int32 i = 0; i < oD.n; i++)
            {
                dX = oD.boxes->x1 - (oD.boxes->x1 % compositor()->globalScale());
                dY = oD.boxes->y1 - (oD.boxes->y1 % compositor()->globalScale());
                dW = oD.boxes->x2 - oD.boxes->x1;
                dW += dW % compositor()->globalScale();
                dH = oD.boxes->y2 - oD.boxes->y1;
                dH += dH % compositor()->globalScale();

                sX = (oD.boxes->x1  - cache.rect.x())/cache.scalingVector.x();
                sX -= sX % compositor()->globalScale();
                sY = (oD.boxes->y1  - cache.rect.y())/cache.scalingVector.y();
                sY -= sY % compositor()->globalScale();
                sW = (oD.boxes->x2 - oD.boxes->x1)/cache.scalingVector.x();
                sW -= sW % compositor()->globalScale();
                sH = (oD.boxes->y2 - oD.boxes->y1)/cache.scalingVector.y();
                sH -= sH % compositor()->globalScale();

                view->paintRectC(
                    oD.p,
                    sX, sY, sW, sH,
                    dX, dY, dW, dH,
                    cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                    1.f,
                    cache.rect.x(), cache.rect.y());

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

                    view->paintRectC(
                        oD.p,
                        oD.boxes->x1 - cache.rect.x(),
                        oD.boxes->y1 - cache.rect.y(),
                        oD.w,
                        oD.h,
                        oD.boxes->x1,
                        oD.boxes->y1,
                        oD.w,
                        oD.h,
                        cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                        1.f,
                        cache.rect.x(), cache.rect.y());

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

                    view->paintRectC(
                        oD.p,
                        x - cache.rect.x(),
                        y - cache.rect.y(),
                        oD.w,
                        oD.h,
                        x,
                        y,
                        oD.w,
                        oD.h,
                        cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                        1.f,
                        cache.rect.x(), cache.rect.y());

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

    for (UInt32 i = 0; i < 7; i++)
        oD.p->setMask(i, nullptr);

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
        LView::LViewPrivate::ViewCache &cache = view->imp()->cache;

        if (!view->isRenderable() || !cache.mapped || cache.occluded)
            return;

        for (UInt32 i = 0; i < 7; i++)
            oD.p->setMask(i, view->getMask(i));

        cache.occluded = true;
        cache.translucent.intersectRegion(oD.newDamageC);
        cache.translucent.subtractRegion(cache.opaqueOverlay);
        oD.boxes = cache.translucent.rects(&oD.n);

        // Draw transulcent rects
        if (cache.scalingEnabled)
        {
            Int32 sX,sY,sW,sH,dX,dY,dW,dH;

            for (Int32 i = 0; i < oD.n; i++)
            {
                dX = oD.boxes->x1 - (oD.boxes->x1 % compositor()->globalScale());
                dY = oD.boxes->y1 - (oD.boxes->y1 % compositor()->globalScale());
                dW = oD.boxes->x2 - oD.boxes->x1;
                dW += dW % compositor()->globalScale();
                dH = oD.boxes->y2 - oD.boxes->y1;
                dH += dH % compositor()->globalScale();

                sX = (oD.boxes->x1  - cache.rect.x())/cache.scalingVector.x();
                sX -= sX % compositor()->globalScale();
                sY = (oD.boxes->y1  - cache.rect.y())/cache.scalingVector.y();
                sY -= sY % compositor()->globalScale();
                sW = (oD.boxes->x2 - oD.boxes->x1)/cache.scalingVector.x();
                sW -= sW % compositor()->globalScale();
                sH = (oD.boxes->y2 - oD.boxes->y1)/cache.scalingVector.y();
                sH -= sH % compositor()->globalScale();

                view->paintRectC(
                    oD.p,
                    sX, sY, sW, sH,
                    dX, dY, dW, dH,
                    cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                    cache.opacity,
                    cache.rect.x(), cache.rect.y());

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

                    view->paintRectC(
                        oD.p,
                        oD.boxes->x1 - cache.rect.x(),
                        oD.boxes->y1 - cache.rect.y(),
                        oD.w,
                        oD.h,
                        oD.boxes->x1,
                        oD.boxes->y1,
                        oD.w,
                        oD.h,
                        cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                        cache.opacity,
                        cache.rect.x(), cache.rect.y());

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

                    view->paintRectC(
                        oD.p,
                        x - cache.rect.x(),
                        y - cache.rect.y(),
                        oD.w,
                        oD.h,
                        x,
                        y,
                        oD.w,
                        oD.h,
                        cache.bufferScaleMatchGlobalScale ? 0.0 : view->bufferScale(),
                        cache.opacity,
                        cache.rect.x(), cache.rect.y());

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

void LScene::LScenePrivate::parentClipping(LView *parent, LRegion *region)
{
    if (!parent)
        return;

    region->clip(LRect(parent->posC(), parent->sizeC()));

    if (parent->parentClippingEnabled())
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
