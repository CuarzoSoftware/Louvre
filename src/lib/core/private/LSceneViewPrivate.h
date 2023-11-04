#ifndef LSCENEVIEWPRIVATE_H
#define LSCENEVIEWPRIVATE_H

#include <private/LViewPrivate.h>
#include <LFramebuffer.h>
#include <LSceneView.h>
#include <LRegion.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <map>
#include <thread>

using namespace Louvre;

LPRIVATE_CLASS(LSceneView)
    LPoint customPos;

    LFramebuffer *fb = nullptr;
    std::list<LOutput*>outputs;
    LRegion input;

    struct ThreadData
    {
        // List of new damage calculated in prev frames
        std::list<LRegion*>prevDamageList;

        // New damage calculated on this frame
        LRegion newDamage;

        // Manually added damage
        LRegion manuallyAddedDamage;

        LRect prevRect;
        LCompositor *c;
        LPainter *p;
        LOutput *o = nullptr;
        Int32 n, w, h;
        LBox *boxes;
        LRegion opaqueTransposedSum;
        LRegion prevExternalExclude;

        bool foundRenderableSaledView;

        // Only for non LScene
        LRegion translucentTransposedSum;
    };

    LRGBAF clearColor = {0,0,0,0};
    std::map<std::thread::id, ThreadData> threadsMap;

    // Quck handle to current output data
    ThreadData *currentThreadData;

    /*
    void cachePass(LView *view, ThreadData *oD);
    void calcNewDamage(LView *view, ThreadData *oD);
    void drawOpaqueDamage(LView *view, ThreadData *oD);
    void drawBackground(ThreadData *oD, bool addToOpaqueSum);
    void drawTranslucentDamage(LView *view, ThreadData *oD);
    void parentClipping(LView *parent, LRegion *region);*/

    inline void clearTmpVariables(ThreadData *oD)
    {
        oD->newDamage.clear();
        oD->opaqueTransposedSum.clear();
        oD->foundRenderableSaledView = false;
    }

    inline void damageAll(ThreadData *oD)
    {
        oD->newDamage.clear();
        oD->newDamage.addRect(fb->rect());
        oD->newDamage.addRect(fb->rect());
    }

    inline void checkRectChange(ThreadData *oD)
    {
        if (oD->prevRect.size() != fb->rect().size())
        {
            damageAll(oD);
            oD->prevRect.setSize(fb->rect().size());
        }
    }

    inline void cachePass(LView *view, ThreadData *oD)
    {
        if (view->type() != Scene)
            for (std::list<LView*>::const_iterator it = view->children().cbegin(); it != view->children().cend(); it++)
                cachePass(*it, oD);

        // Quick view cache handle to reduce verbosity
        LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

        view->imp()->repaintCalled = false;

        // Quick output data handle
        cache->voD = &view->imp()->threadsMap[std::this_thread::get_id()];
        cache->voD->o = oD->o;

        // Cache mapped call
        cache->mapped = view->mapped();

        // Cache view rect
        cache->rect.setPos(view->pos());
        cache->rect.setSize(view->size());

        cache->scalingVector = view->scalingVector();
        cache->scalingEnabled = (view->scalingEnabled() || view->parentScalingEnabled()) && cache->scalingVector != LSizeF(1.f, 1.f);

        if (cache->mapped && cache->scalingEnabled)
            oD->foundRenderableSaledView = true;
    }

    inline void calcNewDamage(LView *view, ThreadData *oD)
    {
        // Children first
        if (view->type() == Scene)
        {
            LSceneView *sceneView = (LSceneView*)view;
            if (view->imp()->cache.scalingEnabled)
                sceneView->render(nullptr);
            else
                sceneView->render(&oD->opaqueTransposedSum);
        }
        else
        {
            for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
                calcNewDamage(*it, oD);
        }

        // Quick view cache handle to reduce verbosity
        LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

        LRegion vRegion;
        vRegion.addRect(cache->rect);

        if (view->clippingEnabled())
            vRegion.clip(view->clippingRect());

        if (view->parent() && view->parentClippingEnabled())
            vRegion.clip(view->parent()->pos(), view->parent()->size());

        // Update view intersected outputs
        for (std::list<LOutput*>::const_iterator it = compositor()->outputs().cbegin(); it != compositor()->outputs().cend(); it++)
        {
            LRegion r = vRegion;
            r.clip((*it)->rect());

            if (!r.empty())
                view->enteredOutput(*it);
            else
                view->leftOutput(*it);
        }

        if (!view->isRenderable())
            return;

        cache->opacity = view->opacity();

        if (view->colorFactor().a <= 0.f || cache->rect.size().area() == 0 || cache->opacity <= 0.f || cache->scalingVector.w() == 0.f || cache->scalingVector.y() == 0.f || (view->clippingEnabled() && view->clippingRect().area() == 0))
            cache->mapped = false;

        bool mappingChanged = cache->mapped != cache->voD->prevMapped;

        if (oD->o && !mappingChanged && !cache->mapped)
        {
            if (view->forceRequestNextFrameEnabled())
                view->requestNextFrame(oD->o);
            return;
        }

        bool opacityChanged = cache->opacity != cache->voD->prevOpacity;

        cache->localRect = LRect(cache->rect.pos() - fb->rect().pos(), cache->rect.size());

        bool rectChanged = cache->localRect != cache->voD->prevLocalRect;

        bool colorFactorChanged = cache->voD->prevColorFactor.r != view->imp()->colorFactor.r ||
                                  cache->voD->prevColorFactor.g != view->imp()->colorFactor.g ||
                                  cache->voD->prevColorFactor.b != view->imp()->colorFactor.b ||
                                  cache->voD->prevColorFactor.a != view->imp()->colorFactor.a;

        // If rect or order changed (set current rect and prev rect as damage)
        if (mappingChanged || rectChanged || cache->voD->changedOrder || opacityChanged || cache->scalingEnabled || colorFactorChanged)
        {
            cache->damage.addRect(cache->rect);

            if (cache->voD->changedOrder)
                cache->voD->changedOrder = false;

            if (mappingChanged)
                cache->voD->prevMapped = cache->mapped;

            if (rectChanged)
            {
                cache->voD->prevRect = cache->rect;
                cache->voD->prevLocalRect = cache->localRect;
            }

            if (opacityChanged)
                cache->voD->prevOpacity = cache->opacity;

            if (colorFactorChanged)
                cache->voD->prevColorFactor = view->imp()->colorFactor;

            if (!cache->mapped)
            {
                oD->newDamage.addRegion(cache->voD->prevClipping);
                return;
            }
        }
        else if (view->damage())
        {
            cache->damage = *view->damage();

            // Scene views already have their damage transposed
            if (view->type() != Scene)
                cache->damage.offset(cache->rect.pos());
        }
        else
        {
            cache->damage.clear();
        }

        // Calculates the non clipped region

        LRegion currentClipping;
        currentClipping.addRect(cache->rect);

        if (view->parentClippingEnabled())
            parentClipping(view->parent(), &currentClipping);

        if (view->clippingEnabled())
            currentClipping.clip(view->clippingRect());

        // Calculates the new exposed view region if parent clipping or clipped region has grown
        LRegion newExposedClipping = currentClipping;
        newExposedClipping.subtractRegion(cache->voD->prevClipping);
        cache->damage.addRegion(newExposedClipping);

        // Add exposed now non clipped region to new output damage
        cache->voD->prevClipping.subtractRegion(currentClipping);
        oD->newDamage.addRegion(cache->voD->prevClipping);

        // Saves current clipped region for next frame
        cache->voD->prevClipping = currentClipping;

        // Clip current damage to current visible region
        cache->damage.intersectRegion(currentClipping);

        // Remove previus opaque region to view damage
        cache->damage.subtractRegion(oD->opaqueTransposedSum);

        // Add clipped damage to new damage
        oD->newDamage.addRegion(cache->damage);

        if (cache->opacity < 1.f || cache->scalingEnabled || view->colorFactor().a < 1.f)
        {
            cache->translucent.clear();
            cache->translucent.addRect(cache->rect);
            cache->opaque.clear();
        }
        else
        {
            // Store tansposed traslucent region
            if (view->translucentRegion())
            {
                cache->translucent = *view->translucentRegion();

                if (view->type() != Scene)
                    cache->translucent.offset(cache->rect.pos());
            }
            else
            {
                cache->translucent.clear();
                cache->translucent.addRect(cache->rect);
            }

            // Store tansposed opaque region
            if (view->opaqueRegion())
            {
                cache->opaque = *view->opaqueRegion();

                if (view->type() != Scene)
                    cache->opaque.offset(cache->rect.pos());
            }
            else
            {
                cache->opaque = cache->translucent;
                cache->opaque.inverse(cache->rect);
            }
        }

        // Clip opaque and translucent regions to current visible region
        cache->opaque.intersectRegion(currentClipping);
        cache->translucent.intersectRegion(currentClipping);

        // Check if view is ocludded
        currentClipping.subtractRegion(oD->opaqueTransposedSum);

        cache->occluded = currentClipping.empty();

        if (oD->o && (!cache->occluded || view->forceRequestNextFrameEnabled()))
            view->requestNextFrame(oD->o);

        // Store sum of previus opaque regions (this will later be clipped when painting opaque and translucent regions)
        cache->opaqueOverlay = oD->opaqueTransposedSum;
        oD->opaqueTransposedSum.addRegion(cache->opaque);
    }

    inline void drawOpaqueDamage(LView *view, ThreadData *oD)
    {
        // Children first
        if (view->type() != Scene)
            for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
                drawOpaqueDamage(*it, oD);

        LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

        if (!view->isRenderable() || !cache->mapped || cache->occluded || cache->opacity < 1.f || view->colorFactor().a < 1.f)
            return;

        cache->opaque.intersectRegion(oD->newDamage);
        cache->opaque.subtractRegion(cache->opaqueOverlay);

        oD->boxes = cache->opaque.boxes(&oD->n);

        oD->p->setColorFactor(view->colorFactor().r,
                              view->colorFactor().g,
                              view->colorFactor().b,
                              view->colorFactor().a);

        if (cache->scalingEnabled)
        {
            for (Int32 i = 0; i < oD->n; i++)
            {
                oD->w = oD->boxes->x2 - oD->boxes->x1;
                oD->h = oD->boxes->y2 - oD->boxes->y1;

                view->paintRect(
                    oD->p,
                    (oD->boxes->x1 - cache->rect.x()) / cache->scalingVector.x(),
                    (oD->boxes->y1  - cache->rect.y()) / cache->scalingVector.y(),
                    oD->w / cache->scalingVector.x(),
                    oD->h / cache->scalingVector.y(),
                    oD->boxes->x1,
                    oD->boxes->y1,
                    oD->w,
                    oD->h,
                    view->bufferScale(),
                    1.f);

                oD->boxes++;
            }
        }
        else
        {
            for (Int32 i = 0; i < oD->n; i++)
            {
                oD->w = oD->boxes->x2 - oD->boxes->x1;
                oD->h = oD->boxes->y2 - oD->boxes->y1;

                view->paintRect(
                    oD->p,
                    oD->boxes->x1 - cache->rect.x(),
                    oD->boxes->y1 - cache->rect.y(),
                    oD->w,
                    oD->h,
                    oD->boxes->x1,
                    oD->boxes->y1,
                    oD->w,
                    oD->h,
                    view->bufferScale(),
                    1.f);

                oD->boxes++;
            }
        }
    }

    inline void drawBackground(ThreadData *oD, bool addToOpaqueSum)
    {
        LRegion backgroundDamage = oD->newDamage;
        backgroundDamage.subtractRegion(oD->opaqueTransposedSum);
        oD->boxes = backgroundDamage.boxes(&oD->n);

        for (Int32 i = 0; i < oD->n; i++)
        {
            oD->p->drawColor(oD->boxes->x1,
                             oD->boxes->y1,
                             oD->boxes->x2 - oD->boxes->x1,
                             oD->boxes->y2 - oD->boxes->y1,
                             clearColor.r,
                             clearColor.g,
                             clearColor.b,
                             clearColor.a);
            oD->boxes++;
        }

        if (addToOpaqueSum)
            oD->opaqueTransposedSum.addRegion(backgroundDamage);
    }

    inline void drawTranslucentDamage(LView *view, ThreadData *oD)
    {
        LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

        if (!view->isRenderable() || !cache->mapped || cache->occluded)
            goto drawChildrenOnly;

        glBlendFunc(view->imp()->sFactor, view->imp()->dFactor);
        oD->p->setColorFactor(view->colorFactor().r,
                              view->colorFactor().g,
                              view->colorFactor().b,
                              view->colorFactor().a);

        cache->occluded = true;
        cache->translucent.intersectRegion(oD->newDamage);
        cache->translucent.subtractRegion(cache->opaqueOverlay);

        oD->boxes = cache->translucent.boxes(&oD->n);

        if (cache->scalingEnabled)
        {
            for (Int32 i = 0; i < oD->n; i++)
            {
                oD->w = oD->boxes->x2 - oD->boxes->x1;
                oD->h = oD->boxes->y2 - oD->boxes->y1;

                view->paintRect(
                    oD->p,
                    (oD->boxes->x1 - cache->rect.x()) / cache->scalingVector.x(),
                    (oD->boxes->y1  - cache->rect.y()) / cache->scalingVector.y(),
                    oD->w / cache->scalingVector.x(),
                    oD->h / cache->scalingVector.y(),
                    oD->boxes->x1,
                    oD->boxes->y1,
                    oD->w,
                    oD->h,
                    view->bufferScale(),
                    cache->opacity);

                oD->boxes++;
            }
        }
        else
        {
            for (Int32 i = 0; i < oD->n; i++)
            {
                oD->w = oD->boxes->x2 - oD->boxes->x1;
                oD->h = oD->boxes->y2 - oD->boxes->y1;

                view->paintRect(
                    oD->p,
                    oD->boxes->x1 - cache->rect.x(),
                    oD->boxes->y1 - cache->rect.y(),
                    oD->w,
                    oD->h,
                    oD->boxes->x1,
                    oD->boxes->y1,
                    oD->w,
                    oD->h,
                    view->bufferScale(),
                    cache->opacity);

                oD->boxes++;
            }
        }

    drawChildrenOnly:
        if (view->type() != Scene)
            for (std::list<LView*>::const_iterator it = view->children().cbegin(); it != view->children().cend(); it++)
                drawTranslucentDamage(*it, oD);
    }

    inline void parentClipping(LView *parent, LRegion *region)
    {
        if (!parent)
            return;

        region->clip(parent->pos(), parent->size());

        if (parent->parentClippingEnabled())
            parentClipping(parent->parent(), region);
    }
};

#endif // LSCENEVIEWPRIVATE_H
