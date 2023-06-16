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

        s->isRenderable = s->mapped() && !s->cursorRole() && s->roleId() != LSurface::Role::Undefined && !s->minimized();

        if (!s->isRenderable)
        {
            s->requestNextFrame();
            continue;
        }

        // 2. Store the current surface rect
        s->currentRectC.setPos(s->rolePosC());
        s->currentRectC.setSize(s->sizeC());

        // 3. Update the surface intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(s->currentRectC))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        // 4. Check if each surface scale matches the global scale.
        s->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        bool rectChanged = s->currentRectC!= s->previousRectC;

        // If changed rect or order (set current rect and prev rect as damage)
        if (rectChanged || s->changedOrder)
        {
            s->currentDamageTransposedC.clear();
            s->currentDamageTransposedC.addRect(s->currentRectC);
            s->currentDamageTransposedC.addRect(s->previousRectC);
            s->changedOrder = false;
            s->previousRectC = s->currentRectC;
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

        if (!s->occluded)
            s->requestNextFrame();

        // Add current transposed opaque region to global sum
        opaqueTransposedCSum.addRegion(s->currentOpaqueTransposedC);
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

        // Draw transulcent rects
        if (s->bufferScaleMatchGlobalScale)
        {
            for (const LRect &d : s->currentOpaqueTransposedC.rects())
                p->drawTextureC(
                    s->texture(),
                    LRect(d.pos() - s->rolePosC(), d.size()),
                    d);
        }
        else
        {
            for (const LRect &d : s->currentOpaqueTransposedC.rects())
                p->drawTextureC(
                    s->texture(),
                    (LRect(d.pos() - s->rolePosC(), d.size())*s->bufferScale())/compositor()->globalScale(),
                    d);
        }
    }

    // Background

    LRegion backgroundDamage = newDamage;
    backgroundDamage.subtractRegion(opaqueTransposedCSum);

    for (const LRect &d : backgroundDamage.rects())
    {
        p->drawColorC(d, 0.05, 0.05, 0.05, 1);
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

        // Draw transulcent rects
        if (s->bufferScaleMatchGlobalScale)
        {
            for (const LRect &d : s->currentTraslucentTransposedC.rects())
                p->drawTextureC(
                    s->texture(),
                    LRect(d.pos() - s->rolePosC(), d.size()),
                    d);
        }
        else
        {
            for (const LRect &d : s->currentTraslucentTransposedC.rects())
                p->drawTextureC(
                    s->texture(),
                    (LRect(d.pos() - s->rolePosC(), d.size())*s->bufferScale())/compositor()->globalScale(),
                    d);
        }
    }

    if (!c->fullscreenSurface)
    {
        LRegion topbarRegion;
        topbarRegion.addRect(LRect(rectC().x(),rectC().y(),rectC().w(),32*compositor()->globalScale()));
        topbarRegion.intersectRegion(newDamage);
        topbarRegion.subtractRegion(opaqueTransposedCSum);

        for (const LRect &d : topbarRegion.rects())
            p->drawColorC(d,1,1,1,0.75);
    }

    newDamage.clear();
}
