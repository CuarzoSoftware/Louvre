#include "Output.h"
#include <LRenderBuffer.h>
#include "TerminalIcon.h"
#include <Compositor.h>
#include <LPainter.h>
#include <Surface.h>
#include <LToplevelRole.h>
#include <LTime.h>
#include <LCursor.h>
#include <LLog.h>
#include <LOpenGL.h>
#include <unistd.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>

Output::Output(const void *params) : LOutput(params){}

static char *joinPaths(const char *path1, const char *path2)
{
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    char *result = (char *)malloc(len1 + len2 + 2);

    // Copy the first path
    snprintf(result, len1 + 1, "%s", path1);

    // Add a '/' if needed
    if (result[len1 - 1] != '/' && path2[0] != '/')
    {
        snprintf(result + len1, 2, "/");
        len1++;
    }

    // Concatenate the second path
    snprintf(result + len1, len2 + 1, "%s", path2);

    return result;
}

void Output::loadWallpaper()
{
    char *path = joinPaths(getenv("HOME"), "/.config/Louvre/wallpaper.jpg");
    LTexture *background = LOpenGL::loadTexture(path);
    free(path);

    if (!background)
    {
        path = joinPaths(compositor()->defaultAssetsPath().c_str(), "wallpaper.png");
        background = LOpenGL::loadTexture(path);
        free(path);
    }

    if (background)
    {
        backgroundTexture = background->copyB(sizeB());
        delete background;
    }
}

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

    loadWallpaper();
    fullDamage();
    repaint();
}

void Output::resizeGL()
{
    Int32 x = 0;

    // Set double scale to outputs with DPI >= 200
    for (Output *output : (std::vector<Output*>&)compositor()->outputs())
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

    loadWallpaper();
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
    LPainter::TextureParams params;
    Compositor *c = (Compositor*)compositor();
    LPainter *p = painter();
    list<Surface*> &surfaces = (list<Surface*>&)compositor()->surfaces();

    p->setAlpha(1.f);
    p->enableCustomTextureColor(false);

    // Creates an LRegion for each output framebuffer
    if (!damageListCreated)
    {
        for (UInt32 i = 0; i < buffersCount() - 1; i++)
            prevDamageList.push_back(new LRegion());

        damageListCreated = true;
    }

    // Damage the entire output if rect changed
    if (lastRect != rect())
    {
        fullDamage();
        lastRect = rect();
    }

    // Check if surface moved under cursor
    if (seat()->pointer()->surfaceAt(cursor()->pos()) != seat()->pointer()->focus())
        seat()->pointer()->pointerMoveEvent(0, 0, false);

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

    /*********************************************************
     ***************** CALC SURFACES DAMAGE ******************
     *********************************************************/

    LRegion opaqueTranslatedCSum;
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
                    if (o->rect().intersects(s->currentRect))
                        s->sendOutputEnterEvent(o);
                    else
                        s->sendOutputLeaveEvent(o);
                }

                s->requestNextFrame(false);
            }

            continue;
        }

        // Store the current surface rect
        s->currentRect.setPos(s->rolePos());
        s->currentRect.setSize(s->size());

        bool clearDamage = true;

        // Update the surface intersected outputs
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(s->currentRect, false))
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

        bool rectChanged = s->currentRect!= s->currentOutputData->previousRect;

        // If rect or order changed (set current rect and prev rect as damage)
        if (rectChanged || s->currentOutputData->changedOrder)
        {
            s->currentDamageTranslated.clear();
            s->currentDamageTranslated.addRect(s->currentRect);
            s->currentDamageTranslated.addRect(s->currentOutputData->previousRect);
            s->currentOutputData->changedOrder = false;
            s->currentOutputData->previousRect = s->currentRect;
        }
        else
        {
            s->currentDamageTranslated = s->damage();
            s->currentDamageTranslated.offset(s->currentRect.pos());
        }

        // Remove overlay opaque region to surface damage
        s->currentDamageTranslated.subtractRegion(opaqueTranslatedCSum);

        // Add clipped damage to new damage
        newDamage.addRegion(s->currentDamageTranslated);

        // Store translated Translucent region
        s->currentTranslucentTranslated = s->translucentRegion();
        s->currentTranslucentTranslated.offset(s->currentRect.pos());

        // Store translated opaque region
        s->currentOpaqueTranslated = s->opaqueRegion();
        s->currentOpaqueTranslated.offset(s->currentRect.pos());

        // Store sum of previus opaque regions
        s->currentOpaqueTranslatedSum = opaqueTranslatedCSum;

        // Check if surface is ocludded
        LRegion ocluddedTest;
        ocluddedTest.addRect(s->currentRect);
        ocluddedTest.subtractRegion(opaqueTranslatedCSum);
        s->occluded = ocluddedTest.empty();

        if (!s->occluded && clearDamage)
            s->requestNextFrame();

        // Add current Translated opaque region to global sum
        opaqueTranslatedCSum.addRegion(s->currentOpaqueTranslated);

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

    /*******************************************************
     ***************** DRAW OPAQUE DAMAGE ******************
     *******************************************************/

    for (list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->currentOpaqueTranslated.intersectRegion(newDamage);
        s->currentOpaqueTranslated.subtractRegion(s->currentOpaqueTranslatedSum);

        params.dstSize = s->currentRect.size();
        params.srcRect = s->srcRect();
        params.pos = s->currentRect.pos();
        params.texture = s->texture();
        params.srcTransform = s->bufferTransform();
        params.srcScale = s->bufferScale();
        p->bindTextureMode(params);
        p->drawRegion(s->currentOpaqueTranslated);
    }

    /****************************************************
     ***************** DRAW BACKGROUND ******************
     ****************************************************/

    LRegion backgroundDamage = newDamage;
    backgroundDamage.subtractRegion(opaqueTranslatedCSum);

    if (backgroundTexture)
    {
        params.texture = backgroundTexture;
        params.dstSize = size();
        params.srcRect = LRectF(0, backgroundTexture->sizeB() - LSize(0, 1));
        params.pos = pos();
        params.srcScale = 1.f;
        params.srcTransform = LFramebuffer::Normal;
        p->bindTextureMode(params);
        p->drawRegion(backgroundDamage);
    }
    else
    {
        p->bindColorMode();

        p->setColor({
            .r = 0.15f,
            .g = 0.25f,
            .b = 0.35f
        });

        p->drawRegion(backgroundDamage);
    }

    glEnable(GL_BLEND);

    /************************************************************
     ***************** DRAW TRANSLUCENT DAMAGE ******************
     ************************************************************/

    for (list<Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->occluded = true;
        s->currentTranslucentTranslated.intersectRegion(newDamage);
        s->currentTranslucentTranslated.subtractRegion(s->currentOpaqueTranslatedSum);

        params.dstSize = s->currentRect.size();
        params.srcRect = s->srcRect();
        params.pos = s->currentRect.pos();
        params.texture = s->texture();
        params.srcTransform = s->bufferTransform();
        params.srcScale = s->bufferScale();
        p->bindTextureMode(params);
        p->drawRegion(s->currentTranslucentTranslated);
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
        p->bindColorMode();
        p->setColor({
            .r = 1.f,
            .g = 1.f,
            .b = 1.f
        });
        p->setAlpha(0.9f);
        p->drawRegion(topbarRegion);

        if (terminalIconTexture)
        {
            LRegion terminalIconRegion;
            terminalIconRegion.addRect(terminalIconRect);
            terminalIconRegion.intersectRegion(topbarRegion);

            if (!terminalIconRegion.empty())
            {
                params.pos = terminalIconRect.pos();
                params.dstSize = terminalIconRect.size();
                params.srcRect = LRect(0, terminalIconTexture->sizeB());
                params.texture = terminalIconTexture;
                params.srcTransform = LFramebuffer::Normal;
                params.srcScale = 1.f;
                p->setAlpha(terminalIconAlpha);
                p->bindTextureMode(params);
                p->drawRect(terminalIconRect);
            }
        }

        if (drawClock)
        {
            params.pos = dstClockRect.pos();
            params.dstSize = dstClockRect.size();
            params.srcRect = LRect(0, c->clock->texture->sizeB());
            params.texture = c->clock->texture;
            params.srcTransform = LFramebuffer::Normal;
            params.srcScale = 1.f;
            p->enableCustomTextureColor(true);
            p->setColor({.r = 0.1f, .g = 0.1f, .b = 0.1f});
            p->setAlpha(1.f);
            p->bindTextureMode(params);
            p->drawRect(dstClockRect);
            p->enableCustomTextureColor(false);
        }
    }

    for (std::list<DestroyedToplevel>::iterator it = c->destroyedToplevels.begin(); it != c->destroyedToplevels.end();)
    {
        float alpha = (256.f - ((float)LTime::ms() - (float)(*it).ms))/256.f;

        if (alpha < 0.f)
            alpha = 0.f;

        params.pos = (*it).rect.pos();
        params.dstSize = (*it).rect.size();
        params.srcRect = LRect(0, (*it).texture->sizeB());
        params.texture = (*it).texture;
        params.srcTransform = LFramebuffer::Normal;
        params.srcScale = 1.f;
        p->bindTextureMode(params);
        p->setAlpha(alpha);
        p->drawRect((*it).rect);

        if (alpha == 0.f)
        {
            delete (*it).texture;
            it = c->destroyedToplevels.erase(it);
        }
        else
        {
            repaint();
            ++it;
        }
    }

    setBufferDamage(&newDamage);
    newDamage.clear();
}
