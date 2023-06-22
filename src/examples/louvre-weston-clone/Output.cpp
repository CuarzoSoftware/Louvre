#include "Output.h"
#include "TerminalIcon.h"
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
    Compositor *c = (Compositor*)compositor();

    if (!c->clock)
        c->clock = new Clock();

    topbarHeight = 32 * compositor()->globalScale();
    terminalIconRectC.setPos(LPoint(9*compositor()->globalScale(), 4*compositor()->globalScale()));
    terminalIconRectC.setSize(LSize(topbarHeight) - LSize(2*terminalIconRectC.pos().y()));
    terminalIconRectC.setPos(rectC().pos() + terminalIconRectC.pos());

    damage.clear();
    newDamage.clear();
    newDamage.addRect(rectC());
}

void Output::initializeGL()
{
    terminalIconTexture = new LTexture(0);

    if (!terminalIconTexture->setDataB(LSize(64,64), 64*4, DRM_FORMAT_ABGR8888, terminalIconPixels()))
    {
        LLog::error("Failed to create terminal icon.");
        delete terminalIconTexture;
        terminalIconTexture = nullptr;
    }

    fullDamage();
    repaint();
}

void Output::resizeGL()
{
    fullDamage();
    repaint();
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

    if (c->clock && c->clock->texture && redrawClock)
    {
        redrawClock = false;
        newDamage.addRect(dstClockRect);
        dstClockRect.setH((32 - 16) * c->globalScale());
        dstClockRect.setW( float(c->clock->texture->sizeB().w()) * float(dstClockRect.h()) / float(c->clock->texture->sizeB().h()));
        dstClockRect.setY(rectC().y() + 9*c->globalScale());
        dstClockRect.setX(rectC().x() + rectC().w() - dstClockRect.w() - 14*c->globalScale());
        newDamage.addRect(dstClockRect);
    }

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
        s->isRenderable = s->mapped() && !s->minimized() && !s->cursorRole();

        // Allow client to update cursor
        if (!s->isRenderable)
        {
            if (s->cursorRole())
            {
                s->setPosC(cursor()->posC());
                for (LOutput *o : compositor()->outputs())
                {
                    if (o->rectC().intersects(s->currentRectC))
                        s->sendOutputEnterEvent(o);
                    else
                        s->sendOutputLeaveEvent(o);
                }

                s->requestNextFrame(false);
            }

            continue;
        }

        // If the scale is equal to the global scale, we avoid performing transformations later
        s->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        // 2. Store the current surface rect
        s->currentRectC.setPos(s->rolePosC());

        if (scale() != compositor()->globalScale())
        {
            s->currentRectC.setX(s->currentRectC.x() - s->currentRectC.x() % compositor()->globalScale());
            s->currentRectC.setY(s->currentRectC.y() - s->currentRectC.y() % compositor()->globalScale());
        }

        s->currentRectC.setSize(s->sizeC());

        // We clear damage only
        bool clearDamage = true;

        // 3. Update the surface intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(s->currentRectC, false))
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

    bool drawClock = false;
    if (c->clock && c->clock->texture)
    {
        LRegion clockRegion;
        clockRegion.addRect(dstClockRect);
        clockRegion.intersectRegion(newDamage);

        if (!clockRegion.empty())
        {
            drawClock = true;
            newDamage.addRect(dstClockRect);
        }
    }

    for (list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->currentOpaqueTransposedC.intersectRegion(newDamage);
        s->currentOpaqueTransposedC.subtractRegion(s->currentOpaqueTransposedCSum);

        boxes = s->currentOpaqueTransposedC.rects(&n);

        // Draw opaque rects
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
                      0.15f,
                      0.25f,
                      0.35f,
                      1.f);
        boxes++;
    }

    glEnable(GL_BLEND);

    // Traslucent

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
                boxes->x1 ,
                boxes->y1,
                w,
                h,
                s->bufferScaleMatchGlobalScale ? 0.0 : s->bufferScale());

            boxes++;
        }
    }

    // Topbar

    if (!fullscreenSurface)
    {
        LRegion topbarRegion;
        topbarRegion.addRect(LRect(rectC().x(),
                                   rectC().y(),
                                   rectC().w(),
                                   topbarHeight));
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
                          0.8f);
            boxes++;
        }

        if (terminalIconTexture)
        {
            LRegion terminalIconRegion;
            terminalIconRegion.addRect(terminalIconRectC);
            terminalIconRegion.intersectRegion(topbarRegion);

            if (!terminalIconRegion.empty())
            {
                p->drawTextureC(terminalIconTexture,
                                LRect(0, terminalIconTexture->sizeB()),
                                terminalIconRectC,
                                0.f,
                                terminalIconAlpha);
            }
        }

        if (drawClock)
        {
            p->drawTextureC(c->clock->texture,
                            LRect(0, c->clock->texture->sizeB()),
                            dstClockRect);
        }
    }

    newDamage.clear();
}
