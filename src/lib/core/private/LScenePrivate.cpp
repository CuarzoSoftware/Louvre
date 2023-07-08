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
        if (oD.o->scale() != compositor()->globalScale())
        {
            view->imp()->currentRectC.setX(view->imp()->currentRectC.x() - view->imp()->currentRectC.x() % compositor()->globalScale());
            view->imp()->currentRectC.setY(view->imp()->currentRectC.y() - view->imp()->currentRectC.y() % compositor()->globalScale());
        }

        // Cache view size
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

        // TODO order changed

        bool rectChanged = view->imp()->currentRectC != view->imp()->currentOutputData->previousRectC;
        bool mappingChanged = view->imp()->currentMapped != view->imp()->currentOutputData->prevMapped;

        // If rect or order changed (set current rect and prev rect as damage)
        if (mappingChanged || rectChanged || view->imp()->currentOutputData->changedOrder)
        {
            view->imp()->currentDamageTransposedC.clear();
            view->imp()->currentDamageTransposedC.addRect(view->imp()->currentRectC);
            view->imp()->currentDamageTransposedC.addRect(view->imp()->currentOutputData->previousRectC);
            view->imp()->currentOutputData->changedOrder = false;
            view->imp()->currentOutputData->prevMapped = view->imp()->currentMapped;
            view->imp()->currentOutputData->previousRectC = view->imp()->currentRectC;
        }
        else
        {
            view->imp()->currentDamageTransposedC = *view->damageC();
            view->imp()->currentDamageTransposedC.offset(view->imp()->currentRectC.pos());
        }

        // Remove previus opaque region to view damage
        view->imp()->currentDamageTransposedC.subtractRegion(oD.opaqueTransposedCSum);

        // Add clipped damage to new damage
        oD.newDamageC.addRegion(view->imp()->currentDamageTransposedC);

        // Store tansposed traslucent region
        view->imp()->currentTraslucentTransposedC = *view->translucentRegionC();
        view->imp()->currentTraslucentTransposedC.offset(view->imp()->currentRectC.pos());

        // Store tansposed opaque region
        view->imp()->currentOpaqueTransposedC = *view->opaqueRegionC();
        view->imp()->currentOpaqueTransposedC.offset(view->imp()->currentRectC.pos());

        // Store sum of previus opaque regions
        view->imp()->currentOpaqueTransposedCSum = oD.opaqueTransposedCSum;

        // Check if view is ocludded
        LRegion ocluddedTest;
        ocluddedTest.addRect(view->imp()->currentRectC);
        ocluddedTest.subtractRegion(oD.opaqueTransposedCSum);
        view->imp()->occluded = ocluddedTest.empty();

        if (!view->imp()->occluded && clearDamage)
            view->requestNextFrame(oD.o);

        // Add current transposed opaque region to global sum
        oD.opaqueTransposedCSum.addRegion(view->imp()->currentOpaqueTransposedC);

        return;
    }

    for (list<LView*>::const_reverse_iterator it = view->children().rbegin(); it != view->children().rend(); it++)
        calcNewDamage(*it, oD);

    calcNewDamage(view, oD, true);
}

void LScene::LScenePrivate::drawOpaqueDamage(LView *view, OutputData &oD, bool force)
{
    if (force)
    {
        if (!view->isRenderable() || !view->imp()->currentMapped || view->imp()->occluded)
            return;

        view->imp()->currentOpaqueTransposedC.intersectRegion(oD.newDamageC);
        view->imp()->currentOpaqueTransposedC.subtractRegion(view->imp()->currentOpaqueTransposedCSum);

        oD.boxes = view->imp()->currentOpaqueTransposedC.rects(&oD.n);

        LSurfaceView *surfaceView = (LSurfaceView*)view;

        // Draw opaque rects
        for (Int32 i = 0; i < oD.n; i++)
        {
            oD.w = oD.boxes->x2 - oD.boxes->x1;
            oD.h = oD.boxes->y2 - oD.boxes->y1;

            oD.p->drawTextureC(
                (LTexture*)surfaceView->surface()->texture(),
                oD.boxes->x1 - view->imp()->currentRectC.x(),
                oD.boxes->y1 - view->imp()->currentRectC.y(),
                oD.w,
                oD.h,
                oD.boxes->x1,
                oD.boxes->y1,
                oD.w,
                oD.h,
                view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale());

            oD.boxes++;
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
                      0.15f,
                      0.25f,
                      0.35f,
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

        LSurfaceView *surfaceView = (LSurfaceView*)view;

        // Draw transulcent rects
        for (Int32 i = 0; i < oD.n; i++)
        {
            oD.w = oD.boxes->x2 - oD.boxes->x1;
            oD.h = oD.boxes->y2 - oD.boxes->y1;

            oD.p->drawTextureC(
                (LTexture*)surfaceView->surface()->texture(),
                oD.boxes->x1 - view->imp()->currentRectC.x(),
                oD.boxes->y1 - view->imp()->currentRectC.y(),
                oD.w,
                oD.h,
                oD.boxes->x1 ,
                oD.boxes->y1,
                oD.w,
                oD.h,
                view->imp()->bufferScaleMatchGlobalScale ? 0.0 : view->scale());

            oD.boxes++;
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

    if (view->inputRegionC().containsPoint(pos - view->posC()))
        v = view;

    return v;
}
