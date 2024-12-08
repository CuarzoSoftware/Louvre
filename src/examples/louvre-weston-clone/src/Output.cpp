#include <LPointerMoveEvent.h>
#include <LScreenshotRequest.h>
#include <LSessionLockRole.h>
#include <LRenderBuffer.h>
#include <LPainter.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LOpenGL.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LDND.h>
#include <LDNDIconRole.h>
#include <LTime.h>
#include <LLog.h>
#include <LUtils.h>
#include <unistd.h>

#include "Compositor.h"
#include "Surface.h"
#include "Output.h"

void Output::loadWallpaper() noexcept
{
    std::unique_ptr<LTexture> background { LOpenGL::loadTexture(std::filesystem::path(getenvString("HOME")) / ".config/Louvre/wallpaper.jpg") };

    if (!background)
        background.reset(LOpenGL::loadTexture(compositor()->defaultAssetsPath() / "wallpaper.png"));

    if (background)
        backgroundTexture.reset(background->copy(sizeB()));
}

void Output::fullDamage() noexcept
{
    Compositor *c { (Compositor*)compositor() };

    if (!c->clock)
        c->clock = std::make_unique<Clock>();
    else
        redrawClock = true;

    terminalIconRect.setPos(LPoint(9, 4));
    terminalIconRect.setSize(LSize(topbarExclusiveZone.size()) - LSize(2*terminalIconRect.pos().y()));
    terminalIconRect.setPos(rect().pos() + terminalIconRect.pos());

    newDamage.clear();
    newDamage.addRect(rect());
}

void Output::initializeGL() noexcept
{
    loadWallpaper();
    fullDamage();
    repaint();
}

void Output::resizeGL() noexcept
{
    Int32 x { 0 };

    // Set double scale to outputs with DPI >= 200
    for (Output *output : (std::vector<Output*>&)compositor()->outputs())
    {
        if (compositor()->graphicBackendId() == LGraphicBackendDRM)
        {
            if (output->dpi() >= 200)
                output->setScale(2);
            else
                output->setScale(1);
        }

        output->setPos(LPoint(x, 0));
        output->fullDamage();
        output->repaint();
        x += output->rect().w();
    }

    if (compositor()->graphicBackendId() == LGraphicBackendDRM)
    {
        backgroundTexture.reset();
        loadWallpaper();
    }
}

void Output::moveGL() noexcept
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

void Output::paintGL() noexcept
{
    LPainter::TextureParams params;
    Compositor *c { static_cast<Compositor*>(compositor()) };
    LPainter *p { painter() };
    std::list<Surface*> &surfaces { (std::list<Surface*>&)compositor()->surfaces() };

    if (fullscreenSurface)
        setContentType(fullscreenSurface->contentType());
    else
        setContentType(LContentTypeNone);

    if (tryFullscreenScanoutIfNoOverlayContent())
        return;

    p->setAlpha(1.f);
    p->enableCustomTextureColor(false);

    // Damage the entire output if rect changed
    if (needsFullRepaint() || lastRect != rect())
    {
        fullDamage();
        lastRect = rect();
    }

    newDamage.addRegion(cursor()->damage(this));

    // Check if surface moved under cursor
    if (seat()->pointer()->surfaceAt(cursor()->pos()) != seat()->pointer()->focus())
        seat()->pointer()->pointerMoveEvent(LPointerMoveEvent());

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

    if (seat()->dnd()->icon())
        seat()->dnd()->icon()->surface()->raise();

    /*********************************************************
     ***************** CALC SURFACES DAMAGE ******************
     *********************************************************/

    LRegion opaqueTranslatedCSum;
    for (std::list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
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

    Int32 age = currentBufferAge();

    if (age > LOUVRE_WESTON_MAX_AGE)
        age = 0;

    if (age == 0)
    {
        newDamage.addRect(rect());
        damageRing[damageRingIndex] = newDamage;
    }
    else
    {
        damageRing[damageRingIndex] = newDamage;

        for (Int32 i = 1; i < age; i++)
        {
            Int32 damageIndex = damageRingIndex - i;

            if (damageIndex < 0)
                damageIndex = LOUVRE_WESTON_MAX_AGE + damageIndex;

            newDamage.addRegion(damageRing[damageIndex]);
        }
    }

    if (damageRingIndex == LOUVRE_WESTON_MAX_AGE - 1)
        damageRingIndex = 0;
    else
        damageRingIndex++;


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

    for (std::list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->currentOpaqueTranslated.intersectRegion(newDamage);
        s->currentOpaqueTranslated.subtractRegion(s->currentOpaqueTranslatedSum);

        params.texture = s->texture();
        params.dstSize = s->currentRect.size();
        params.srcRect = s->srcRect();
        params.pos = s->currentRect.pos();
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
        params.texture = backgroundTexture.get();
        params.dstSize = size();
        params.srcRect = LRectF(0, backgroundTexture->sizeB());
        params.pos = pos();
        params.srcTransform = LTransform::Normal;
        params.srcScale = 1.f;
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

    for (std::list<Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        Surface *s = *it;

        if (!s->isRenderable || s->occluded)
            continue;

        s->occluded = true;
        s->currentTranslucentTranslated.intersectRegion(newDamage);
        s->currentTranslucentTranslated.subtractRegion(s->currentOpaqueTranslatedSum);

        params.texture = s->texture();
        params.dstSize = s->currentRect.size();
        params.srcRect = s->srcRect();
        params.pos = s->currentRect.pos();
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
                                   topbarExclusiveZone.size()));
        topbarRegion.intersectRegion(newDamage);
        p->bindColorMode();
        p->setColor({
            .r = 1.f,
            .g = 1.f,
            .b = 1.f
        });
        p->setAlpha(0.9f);
        p->drawRegion(topbarRegion);

        if (c->terminalIconTexture)
        {
            LRegion terminalIconRegion;
            terminalIconRegion.addRect(terminalIconRect);
            terminalIconRegion.intersectRegion(topbarRegion);

            if (!terminalIconRegion.empty())
            {
                params.texture = c->terminalIconTexture.get();
                params.dstSize = terminalIconRect.size();
                params.srcRect = LRect(0, c->terminalIconTexture->sizeB());
                params.pos = terminalIconRect.pos();
                params.srcTransform = LTransform::Normal;
                params.srcScale = 1.f;
                p->setAlpha(terminalIconAlpha);
                p->bindTextureMode(params);
                p->drawRect(terminalIconRect);
            }
        }

        if (drawClock)
        {
            params.texture = c->clock->texture.get();
            params.dstSize = dstClockRect.size();
            params.srcRect = LRect(0, c->clock->texture->sizeB());
            params.pos = dstClockRect.pos();
            params.srcTransform = LTransform::Normal;
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

        params.texture = (*it).texture;
        params.dstSize = (*it).rect.size();
        params.srcRect = LRect(0, (*it).texture->sizeB());
        params.pos = (*it).rect.pos();
        params.srcTransform = LTransform::Normal;
        params.srcScale = 1.f;
        p->bindTextureMode(params);
        p->setAlpha(alpha);
        p->drawRect((*it).rect);

        if (alpha == 0.f)
        {
            delete (*it).texture;
            it = c->destroyedToplevels.erase(it);
            for (LOutput *o : compositor()->outputs())
                static_cast<Output*>(o)->lastRect.setX(-1);
        }
        else
        {
            repaint();
            ++it;
        }
    }

    for (LScreenshotRequest *ssReq : screenshotRequests())
        ssReq->accept(true);

    setBufferDamage(&newDamage);
    newDamage.clear();
}

static bool hasMappedChildren(LSurface *surface)
{
    if (!surface)
        return false;

    for (LSurface *child : surface->children())
        if (child->mapped())
            return true;

    return false;
}

bool Output::tryFullscreenScanoutIfNoOverlayContent() noexcept
{
    LSurface *fullscreenSurface { nullptr };

    /* Try with a sessionLock surface or fullscreen toplevel */
    if (sessionLockRole() && sessionLockRole()->surface()->mapped())
        fullscreenSurface = sessionLockRole()->surface();
    else if (this->fullscreenSurface && this->fullscreenSurface->mapped())
        fullscreenSurface = this->fullscreenSurface;

    if (!fullscreenSurface
        || !static_cast<Compositor*>(compositor())->destroyedToplevels.empty()
        || (cursor()->visible() && !cursor()->hwCompositingEnabled(this))
        || !screenshotRequests().empty()
        || hasMappedChildren(fullscreenSurface))
        return false;

    const bool ret { setCustomScanoutBuffer(fullscreenSurface->texture()) };

    if (ret)
        fullscreenSurface->requestNextFrame(true);

    return ret;
}

