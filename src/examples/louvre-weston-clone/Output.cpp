#include "Output.h"
#include <Compositor.h>
#include <LPainter.h>
#include <Surface.h>
#include <LToplevelRole.h>
#include <LTime.h>
#include <LCursor.h>
#include <LLog.h>

Output::Output():LOutput(){}

void Output::fullDamage()
{
    damage.clear();
    damage.addRect(rectC());
    newDamage.clear();
    newDamage.addRect(rectC());
}

void Output::initializeGL()
{
    fullDamage();
    paintGL();
}

void Output::resizeGL()
{
    fullDamage();
    paintGL();
}

void repaintParent(LSurface *s)
{
    if (s)
    {
        s->requestNextFrame();
        repaintParent(s->parent());
    }
}

void repaintChildren(LSurface *s)
{
    for (LSurface *c : s->children())
    {
        c->requestNextFrame();
        repaintChildren(c);
    }
}

void Output::paintGL()
{
    Int32 n, w, h;
    LRect rect;
    LBox *boxes;

    if (lastRectC != rectC())
    {
        fullDamage();
        lastRectC = rectC();
    }

    // Check if surface moved under cursor
    if (seat()->pointer()->surfaceAtC(cursor()->posC()) != seat()->pointer()->focusSurface())
        seat()->pointer()->pointerPosChangeEvent(
            cursor()->posC().x(),
            cursor()->posC().y());

    Compositor *c = (Compositor*)compositor();
    LPainter *p = painter();
    list<Surface*> &surfaces = (list<Surface*>&)compositor()->surfaces();

    if (seat()->dndManager()->icon())
        compositor()->raiseSurface(seat()->dndManager()->icon()->surface());

    /* In this pass we:
     * - 1. Calculate the new output damage.
     * - 2. Mark each surface as ocludded by default.
     * - 3. Update the outputs each surface intersects.
     * - 4. Check if each surface scale matches the global scale.
     * - 5. Check if each surface changed its position or size.
     * - 3. Transpose each surface damage.
     * - 4. Transpose each surface traslucent region.
     * - 5. Transpose each surface opaque region.
     * - 6. Sum the surfaces transposed opaque region. */
    LRegion opaqueTransposedCSum;
    for (list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        // Create or get quick access to the current output data
        s->currentOutputData = &s->outputsMap[this];

        // Check if meets conditions to be rendered
        s->isRenderable = s->mapped() && !s->cursorRole() && s->roleId() != LSurface::Role::Undefined && !s->minimized();

        // Allow client to update cursor
        if (!s->isRenderable)
        {
            s->requestNextFrame();
            continue;
        }

        // If the scale is equal to the global scale, we avoid performing transformations later
        s->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        // 2. Store the current surface rect
        s->currentRectC.setPos(s->rolePosC());
        s->currentRectC.setSize(s->sizeC());

        // We clear damage only
        bool clearDamage = true;

        // 3. Update the surface intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(s->currentRectC))
            {
                s->sendOutputEnterEvent(o);

                if (o != this && (s->outputsMap[o].lastRenderedDamageId < s->damageId()))
                {
                    clearDamage = false;
                    o->repaint();
                }
            }
            else
                s->sendOutputLeaveEvent(o);
        }

        bool rectChanged = s->currentRectC!= s->currentOutputData->previousRectC;

        // If rect or order changed (set current rect and prev rect as damage)
        if (rectChanged || s->currentOutputData->changedOrder)
        {
            s->currentDamageTransposedC.clear();
            s->currentDamageTransposedC.addRect(s->currentRectC);
            s->currentDamageTransposedC.addRect(s->currentOutputData->previousRectC);
            s->currentOutputData->changedOrder = false;
            s->currentOutputData->previousRectC = s->currentRectC;
        }
        else
        {
            s->currentDamageTransposedC = s->damagesC();
            s->currentDamageTransposedC.offset(s->currentRectC.pos());
        }

        // Remove previus opaque region to surface damage
        s->currentDamageTransposedC.subtractRegion(opaqueTransposedCSum);

        // Add clipped damage to new damage
        newDamage.addRegion(s->currentDamageTransposedC);

        // Store tansposed traslucent region
        s->currentTraslucentTransposedC = s->translucentRegionC();
        s->currentTraslucentTransposedC.offset(s->currentRectC.pos());

        // Store tansposed opaque region
        s->currentOpaqueTransposedC = s->opaqueRegionC();
        s->currentOpaqueTransposedC.offset(s->currentRectC.pos());

        // Store sum of previus opaque regions
        s->currentOpaqueTransposedCSum = opaqueTransposedCSum;

        // Check if surface is ocludded
        LRegion ocluddedTest;
        ocluddedTest.addRect(s->currentRectC);
        ocluddedTest.subtractRegion(opaqueTransposedCSum);
        s->occluded = ocluddedTest.empty();

        if (!s->occluded && clearDamage)
            s->requestNextFrame();

        // Add current transposed opaque region to global sum
        opaqueTransposedCSum.addRegion(s->currentOpaqueTransposedC);

        s->currentOutputData->lastRenderedDamageId = s->damageId();
    }

    glDisable(GL_BLEND);

    // Save new damage for next frame and add old damage to current damage
    LRegion oldDamage = damage;
    damage = newDamage;
    newDamage.addRegion(oldDamage);

    for (list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->currentOpaqueTransposedC.intersectRegion(newDamage);
        s->currentOpaqueTransposedC.subtractRegion(s->currentOpaqueTransposedCSum);

        boxes = s->currentOpaqueTransposedC.rects(&n);

        // Draw transulcent rects
        if (s->bufferScaleMatchGlobalScale)
        {
            for (Int32 i = 0; i < n; i++)
            {
                w = boxes->x2 - boxes->x1;
                h = boxes->y2 - boxes->y1;

                p->drawTextureC(
                    s->texture(),
                    boxes->x1 - s->currentRectC.x(),
                    boxes->y1 - s->currentRectC.y(),
                    w,
                    h,
                    boxes->x1,
                    boxes->y1,
                    w,
                    h,
                    s->bufferScaleMatchGlobalScale ? 0.0 : s->bufferScale());

                boxes++;
            }
        }
    }

    // Background

    LRegion backgroundDamage = newDamage;
    backgroundDamage.subtractRegion(opaqueTransposedCSum);
    boxes = backgroundDamage.rects(&n);

    for (Int32 i = 0; i < n; i++)
    {
        p->drawColorC(boxes->x1,
                      boxes->y1,
                      boxes->x2 - boxes->x1,
                      boxes->y2 - boxes->y1,
                      0.05f,
                      0.05f,
                      0.05f,
                      1.f);
        boxes++;
    }

    glEnable(GL_BLEND);

    for (list<Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->occluded = true;
        s->currentTraslucentTransposedC.intersectRegion(newDamage);
        s->currentTraslucentTransposedC.subtractRegion(s->currentOpaqueTransposedCSum);

        boxes = s->currentTraslucentTransposedC.rects(&n);

        // Draw transulcent rects
        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->x2 - boxes->x1;
            h = boxes->y2 - boxes->y1;

            p->drawTextureC(
                s->texture(),
                boxes->x1 - s->currentRectC.x(),
                boxes->y1 - s->currentRectC.y(),
                w,
                h,
                boxes->x1,
                boxes->y1,
                w,
                h,
                s->bufferScaleMatchGlobalScale ? 0.0 : s->bufferScale());

            boxes++;
        }
    }

    // Topbar

    if (!c->fullscreenSurface)
    {
        LRegion topbarRegion;
        topbarRegion.addRect(LRect(rectC().x(),rectC().y(),rectC().w(),32*compositor()->globalScale()));
        topbarRegion.intersectRegion(newDamage);
        boxes = topbarRegion.rects(&n);

        for (Int32 i = 0; i < n; i++)
        {
            p->drawColorC(boxes->x1,
                          boxes->y1,
                          boxes->x2 - boxes->x1,
                          boxes->y2 - boxes->y1,
                          1.0f,
                          1.0f,
                          1.0f,
                          0.75f);
            boxes++;
        }
    }

    newDamage.clear();
}
