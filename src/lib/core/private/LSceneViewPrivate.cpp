#include <private/LSceneViewPrivate.h>
#include <private/LViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LSurfaceView.h>
#include <LLog.h>
#include <LFramebuffer.h>
#include <pixman.h>

void LSceneView::LSceneViewPrivate::calcNewDamage(LView *view, ThreadData *oD)
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

    /************** Update intersected outputs **************/

    LRegion currentClipping(cache->rect);

    if (view->parentClippingEnabled())
        parentClipping(view->parent(), &currentClipping);

    if (view->clippingEnabled())
        currentClipping.clip(view->clippingRect());

    LRegion intersectedRegionTmp;
    for (std::list<LOutput*>::const_iterator it = compositor()->outputs().cbegin(); it != compositor()->outputs().cend(); it++)
    {
        const LRect &rect = (*it)->rect();
        pixman_region32_intersect_rect(&intersectedRegionTmp.m_region, &currentClipping.m_region, rect.x(), rect.y(), rect.w(), rect.h());

        if (!intersectedRegionTmp.empty())
            view->enteredOutput(*it);
        else
            view->leftOutput(*it);
    }

    /********************************************************/

    if (!view->isRenderable())
        return;

    cache->opacity = view->opacity();

    if (view->imp()->colorFactor.a <= 0.f || cache->rect.size().area() == 0 || cache->opacity <= 0.f || cache->scalingVector.w() == 0.f || cache->scalingVector.y() == 0.f || (view->clippingEnabled() && view->clippingRect().area() == 0))
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

    bool colorFactorChanged = cache->voD->prevColorFactorEnabled != view->imp()->colorFactorEnabled;

    if (!colorFactorChanged && view->imp()->colorFactorEnabled)
    {
        colorFactorChanged = cache->voD->prevColorFactor.r != view->imp()->colorFactor.r ||
                              cache->voD->prevColorFactor.g != view->imp()->colorFactor.g ||
                              cache->voD->prevColorFactor.b != view->imp()->colorFactor.b ||
                              cache->voD->prevColorFactor.a != view->imp()->colorFactor.a;
    }

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
        {
            cache->voD->prevColorFactorEnabled = view->imp()->colorFactorEnabled;
            cache->voD->prevColorFactor = view->imp()->colorFactor;
        }

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

    // Calculates the new exposed view region if parent clipping or clipped region has grown
    LRegion newExposedClipping;
    pixman_region32_subtract(&newExposedClipping.m_region, &currentClipping.m_region, &cache->voD->prevClipping.m_region);
    cache->damage.addRegion(newExposedClipping);

    // Add exposed now non clipped region to new output damage
    cache->voD->prevClipping.subtractRegion(currentClipping);
    oD->newDamage.addRegion(cache->voD->prevClipping);

    // Saves current clipped region for next frame
    cache->voD->prevClipping = currentClipping;

    // Clip current damage to current visible region
    cache->damage.intersectRegion(currentClipping);

    // Remove above opaque region to view damage
    cache->damage.subtractRegion(oD->opaqueTransposedSum);

    // Add clipped damage to new damage
    oD->newDamage.addRegion(cache->damage);

    cache->isFullyTrans = cache->opacity < 1.f || cache->scalingEnabled || view->colorFactor().a < 1.f;

    if (cache->isFullyTrans)
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
            pixman_box32_t r;
            r.x1 = cache->rect.x();
            r.x2 = r.x1 + cache->rect.w();
            r.y1 = cache->rect.y();
            r.y2 = r.y1 + cache->rect.h();
            pixman_region32_inverse(&cache->opaque.m_region, &cache->translucent.m_region, &r);
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

void LSceneView::LSceneViewPrivate::drawOpaqueDamage(LView *view, ThreadData *oD)
{
    // Children first
    if (view->type() != Scene)
        for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
            drawOpaqueDamage(*it, oD);

    LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

    if (!view->isRenderable() || !cache->mapped || cache->occluded || cache->isFullyTrans)
        return;

    cache->opaque.intersectRegion(oD->newDamage);
    cache->opaque.subtractRegion(cache->opaqueOverlay);

    oD->boxes = cache->opaque.boxes(&oD->n);

    if (view->imp()->colorFactorEnabled)
    {
        oD->p->imp()->shaderSetColorFactor(view->imp()->colorFactor.r,
                             view->imp()->colorFactor.g,
                             view->imp()->colorFactor.b,
                             view->imp()->colorFactor.a);
    }
    else
        oD->p->imp()->shaderSetColorFactorEnabled(0);

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

void LSceneView::LSceneViewPrivate::drawBackground(ThreadData *oD, bool addToOpaqueSum)
{
    LRegion backgroundDamage;
    pixman_region32_subtract(&backgroundDamage.m_region, &oD->newDamage.m_region, &oD->opaqueTransposedSum.m_region);
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

void LSceneView::LSceneViewPrivate::drawTranslucentDamage(LView *view, ThreadData *oD)
{
    LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

    if (!view->isRenderable() || !cache->mapped || cache->occluded)
        goto drawChildrenOnly;

    glBlendFunc(view->imp()->sFactor, view->imp()->dFactor);

    if (view->imp()->colorFactorEnabled)
    {
        oD->p->imp()->shaderSetColorFactor(view->imp()->colorFactor.r,
                              view->imp()->colorFactor.g,
                              view->imp()->colorFactor.b,
                              view->imp()->colorFactor.a);
    }
    else
        oD->p->imp()->shaderSetColorFactorEnabled(0);

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

void LSceneView::LSceneViewPrivate::parentClipping(LView *parent, LRegion *region)
{
    if (!parent)
        return;

    region->clip(parent->pos(), parent->size());

    if (parent->parentClippingEnabled())
        parentClipping(parent->parent(), region);
}
