#include "Output.h"
#include "TerminalIcon.h"
#include <Compositor.h>
#include <LPainter.h>
#include <Surface.h>
#include <LToplevelRole.h>
#include <LTime.h>
#include <LCursor.h>
#include <LLog.h>
#include <LOpenGL.h>

Output::Output():LOutput(){}

void Output::fullDamage()
{
    Compositor *c = (Compositor*)compositor();

    if (!c->clock)
        c->clock = new Clock();
    else
        redrawClock = true;

    topbarHeight = 32;
    terminalIconRect.setPos(LPoint(9, 4));
    terminalIconRect.setSize(LSize(topbarHeight) - LSize(2*terminalIconRect.pos().y()));
    terminalIconRect.setPos(rect().pos() + terminalIconRect.pos());

    newDamage.clear();
    newDamage.addRect(rect());
}

void Output::initializeGL()
{
    terminalIconTexture = new LTexture();

    if (!terminalIconTexture->setDataB(LSize(64,64), 64*4, DRM_FORMAT_ABGR8888, terminalIconPixels()))
    {
        LLog::error("Failed to create terminal icon.");
        delete terminalIconTexture;
        terminalIconTexture = nullptr;
    }

    char wallpaperPath[256];
    sprintf(wallpaperPath, "%s/.config/Louvre/wallpaper.jpg", getenv("HOME"));
    LTexture *background = LOpenGL::loadTexture(wallpaperPath);

    if (background)
    {
        backgroundTexture = background->copyB(sizeB());
        LLog::debug("Background texture size %d %d", backgroundTexture->sizeB().w(), backgroundTexture->sizeB().h());
        delete background;
    }

    fullDamage();
    repaint();
}

void Output::resizeGL()
{
    Int32 x = 0;
    // Set double scale to outputs with DPI >= 200
    for (Output *output : (std::list<Output*>&)compositor()->outputs())
    {
        if (output->dpi() >= 200)
            output->setScale(2);
        else
            output->setScale(1);

        output->setPos(LPoint(x, 0));
        output->fullDamage();
        output->repaint();
        x += output->rect().w();
    }

    if (backgroundTexture)
    {
        delete backgroundTexture;
        backgroundTexture = nullptr;
    }

    char wallpaperPath[256];
    sprintf(wallpaperPath, "%s/.config/Louvre/wallpaper.jpg", getenv("HOME"));
    LTexture *background = LOpenGL::loadTexture(wallpaperPath);

    if (background)
    {
        backgroundTexture = background->copyB(sizeB()/50);
        LLog::debug("Background texture size %d %d", backgroundTexture->sizeB().w(), backgroundTexture->sizeB().h());
        delete background;
    }
}

void Output::moveGL()
{
    fullDamage();
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
    if (!damageListCreated)
    {
        for (UInt32 i = 0; i < buffersCount() - 1; i++)
            prevDamageList.push_back(new LRegion());

        damageListCreated = true;
    }

    Int32 n, w, h;
    LBox *boxes;

    if (lastRect != rect())
    {
        fullDamage();
        lastRect = rect();
    }

    // Check if surface moved under cursor
    if (seat()->pointer()->surfaceAt(cursor()->pos()) != seat()->pointer()->focusSurface())
        seat()->pointer()->pointerPosChangeEvent(
            cursor()->pos().x(),
            cursor()->pos().y());

    Compositor *c = (Compositor*)compositor();
    LPainter *p = painter();
    list<Surface*> &surfaces = (list<Surface*>&)compositor()->surfaces();

    if (c->clock && c->clock->texture && redrawClock)
    {
        redrawClock = false;
        newDamage.addRect(dstClockRect);
        dstClockRect.setH(32 - 16);
        dstClockRect.setW( float(c->clock->texture->sizeB().w()) * float(dstClockRect.h()) / float(c->clock->texture->sizeB().h()));
        dstClockRect.setY(rect().y() + 9);
        dstClockRect.setX(rect().x() + rect().w() - dstClockRect.w() - 14);
        newDamage.addRect(dstClockRect);
    }

    if (seat()->dndManager()->icon())
        seat()->dndManager()->icon()->surface()->raise();

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
                s->setPos(cursor()->pos());
                for (LOutput *o : compositor()->outputs())
                {
                    if (o->rect().intersects(s->currentRectC))
                        s->sendOutputEnterEvent(o);
                    else
                        s->sendOutputLeaveEvent(o);
                }

                s->requestNextFrame(false);
            }

            continue;
        }

        // If the scale is equal to the global scale, we avoid performing transformations later
        //s->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        // 2. Store the current surface rect
        s->currentRectC.setPos(s->rolePos());
        s->currentRectC.setSize(s->size());

        // We clear damage only
        bool clearDamage = true;

        // 3. Update the surface intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(s->currentRectC, false))
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
            s->currentDamageTransposedC = s->damage();
            s->currentDamageTransposedC.offset(s->currentRectC.pos());
        }

        // Remove previus opaque region to surface damage
        s->currentDamageTransposedC.subtractRegion(opaqueTransposedCSum);

        // Add clipped damage to new damage
        newDamage.addRegion(s->currentDamageTransposedC);

        // Store tansposed traslucent region
        s->currentTraslucentTransposedC = s->translucentRegion();
        s->currentTraslucentTransposedC.offset(s->currentRectC.pos());

        // Store tansposed opaque region
        s->currentOpaqueTransposedC = s->opaqueRegion();
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

    for (DestroyedToplevel &destroyed : c->destroyedToplevels)
    {
        newDamage.addRect(destroyed.rect);

        for (LOutput *o : destroyed.outputs)
            o->repaint();
    }

    glDisable(GL_BLEND);

    // Save new damage for next frame and add old damage to current damage
    if (buffersCount() > 1)
    {
        LRegion oldDamage = *prevDamageList.front();

        for (std::list<LRegion*>::iterator it = std::next(prevDamageList.begin()); it != prevDamageList.end(); it++)
            oldDamage.addRegion(*(*it));

        LRegion *front = prevDamageList.front();
        prevDamageList.pop_front();
        prevDamageList.push_back(front);

        *front = newDamage;
        newDamage.addRegion(oldDamage);
    }

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

            p->drawTexture(
                s->texture(),
                boxes->x1 - s->currentRectC.x(),
                boxes->y1 - s->currentRectC.y(),
                w,
                h,
                boxes->x1,
                boxes->y1,
                w,
                h,
                s->bufferScale());

            boxes++;
        }
    }

    // Background
    LRegion backgroundDamage = newDamage;
    backgroundDamage.subtractRegion(opaqueTransposedCSum);
    boxes = backgroundDamage.rects(&n);

    if (backgroundTexture)
    {
        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->x2 - boxes->x1;
            h = boxes->y2 - boxes->y1;

            p->drawTexture(backgroundTexture,
                            boxes->x1 - pos().x(),
                            boxes->y1 - pos().y(),
                            w,
                            h,
                            boxes->x1,
                            boxes->y1,
                            w,
                            h,
                            scale());
            boxes++;
        }
    }
    else
    {
        for (Int32 i = 0; i < n; i++)
        {
            p->drawColor(boxes->x1,
                          boxes->y1,
                          boxes->x2 - boxes->x1,
                          boxes->y2 - boxes->y1,
                          0.15f,
                          0.25f,
                          0.35f,
                          1.f);
            boxes++;
        }
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

            p->drawTexture(
                s->texture(),
                boxes->x1 - s->currentRectC.x(),
                boxes->y1 - s->currentRectC.y(),
                w,
                h,
                boxes->x1 ,
                boxes->y1,
                w,
                h,
                s->bufferScale());

            boxes++;
        }
    }

    // Topbar

    if (!fullscreenSurface)
    {
        LRegion topbarRegion;
        topbarRegion.addRect(LRect(rect().x(),
                                   rect().y(),
                                   rect().w(),
                                   topbarHeight));
        topbarRegion.intersectRegion(newDamage);
        boxes = topbarRegion.rects(&n);

        for (Int32 i = 0; i < n; i++)
        {
            p->drawColor(boxes->x1,
                          boxes->y1,
                          boxes->x2 - boxes->x1,
                          boxes->y2 - boxes->y1,
                          1.0f,
                          1.0f,
                          1.0f,
                          0.9f);
            boxes++;
        }

        if (terminalIconTexture)
        {
            LRegion terminalIconRegion;
            terminalIconRegion.addRect(terminalIconRect);
            terminalIconRegion.intersectRegion(topbarRegion);

            if (!terminalIconRegion.empty())
            {
                p->drawTexture(terminalIconTexture,
                                LRect(0, terminalIconTexture->sizeB()),
                                terminalIconRect,
                                1.f,
                                terminalIconAlpha);
            }
        }

        if (drawClock)
        {
            p->drawTexture(c->clock->texture,
                            LRect(0, c->clock->texture->sizeB()),
                            dstClockRect);
        }
    }

    for (std::list<DestroyedToplevel>::iterator it = c->destroyedToplevels.begin(); it != c->destroyedToplevels.end(); it++)
    {
        float alpha = (256.f - ((float)LTime::ms() - (float)(*it).ms))/256.f;

        if (alpha < 0.f)
            alpha = 0.f;

        p->drawTexture((*it).texture,
                        LRect(0, (*it).texture->sizeB()),
                        (*it).rect,
                        1.f,
                        alpha);

        if (alpha == 0.f)
        {
            delete (*it).texture;
            it = c->destroyedToplevels.erase(it);
        }
        else
        {
            repaint();
        }
    }

    setBufferDamage(newDamage);
    newDamage.clear();
}
