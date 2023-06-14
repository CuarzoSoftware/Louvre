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
    bool passFullscreen = false;

    if (seat()->dndManager()->icon())
        compositor()->raiseSurface(seat()->dndManager()->icon()->surface());

    // Calc surface damages
    list<Surface*>::iterator fullscreenIt;
    for (list<Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        Surface *s = *it;

        if (!s->mapped() || s->cursorRole()|| s->roleId() == LSurface::Role::Undefined || s->minimized())
        {
            s->requestNextFrame();
            continue;
        }

        s->occluded = true;

        if (s == c->fullscreenSurface)
        {
            fullscreenIt = it;
            passFullscreen = true;
        }

        // Skip until find fullscreen surface
        if (c->fullscreenSurface && ! passFullscreen)
            continue;

        // Currrent surface rect
        LPoint sp = s->rolePosC();
        LRect rC = LRect(sp, s->sizeC());

        // Update intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rectC().intersects(rC))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        s->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        bool rectChanged = rC != s->lastRect;

        // If changed rect or order (set current rect and prev rect as damage)
        if (rectChanged || s->changedOrder)
        {
            newDamage.addRect(rC);
            newDamage.addRect(s->lastRect);
            s->changedOrder = false;
            s->lastRect = rC;
        }
        else
        {
            s->damage = s->damagesC();
            s->damage.offset(rC.pos());
            newDamage.addRegion(s->damage);
        }
    }

    glDisable(GL_BLEND);

    LRegion oldDamage = damage;
    oldDamage.subtractRegion(newDamage);
    damage = newDamage;
    newDamage.addRegion(oldDamage);

    if (!passFullscreen)
        fullscreenIt = surfaces.begin();

    LRegion opaqueMask = newDamage;
    LRegion opaqueSum;

    for (list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        if (!s->mapped() || s->roleId() == LSurface::Role::Cursor || s->roleId() == LSurface::Role::Undefined || s->minimized())
            continue;

        s->opaque = s->opaqueRegionC();
        s->opaque.offset(s->rolePosC());
        s->opaque.intersectRegion(opaqueMask);

        if (!s->opaque.empty())
            s->occluded = false;

        // Draw transulcent rects
        if (s->bufferScaleMatchGlobalScale)
        {
            for (const LRect &d : s->opaque.rects())
                p->drawTextureC(
                    s->texture(),
                    LRect(d.pos() - s->rolePosC(), d.size()),
                    d);
        }
        else
        {
            for (const LRect &d : s->opaque.rects())
                p->drawTextureC(
                    s->texture(),
                    (LRect(d.pos() - s->rolePosC(), d.size())*s->bufferScale())/compositor()->globalScale(),
                    d);
        }

        opaqueMask.subtractRegion(s->opaque);
        opaqueSum.addRegion(s->opaque);
        s->opaque = opaqueSum;

        if (s == c->fullscreenSurface)
            break;
    }

    // Background

    if (!c->fullscreenSurface)
    {
        for (const LRect &d : opaqueMask.rects())
        {
            p->drawColorC(d, 0.05, 0.05, 0.05, 1);
        }
    }

    glEnable(GL_BLEND);

    LRegion trans;

    for (list<Surface*>::iterator it = fullscreenIt; it != surfaces.end(); it++)
    {
        Surface *s = *it;

        if (!s->mapped() || s->cursorRole() || s->roleId() == LSurface::Role::Undefined || s->minimized())
            continue;

        trans = s->translucentRegionC();
        trans.offset(s->rolePosC());
        trans.intersectRegion(newDamage);
        trans.subtractRegion(s->opaque);

        if (!trans.empty())
            s->occluded = false;

        // Draw transulcent rects
        if (s->bufferScaleMatchGlobalScale)
        {
            for (const LRect &d : trans.rects())
                p->drawTextureC(
                    s->texture(),
                    LRect(d.pos() - s->rolePosC(), d.size()),
                    d);
        }
        else
        {
            for (const LRect &d : trans.rects())
                p->drawTextureC(
                    s->texture(),
                    (LRect(d.pos() - s->rolePosC(), d.size())*s->bufferScale())/compositor()->globalScale(),
                    d);
        }

        if (!s->occluded || s->subsurface())
            s->requestNextFrame();
    }

    if (!c->fullscreenSurface)
    {
        LRegion topbarRegion;
        topbarRegion.addRect(LRect(rectC().x(),rectC().y(),rectC().w(),32*compositor()->globalScale()));
        topbarRegion.intersectRegion(newDamage);

        for (const LRect &d : topbarRegion.rects())
            p->drawColorC(d,1,1,1,0.75);
    }

    newDamage.clear();
}
