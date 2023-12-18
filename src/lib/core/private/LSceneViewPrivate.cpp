#include <private/LSceneViewPrivate.h>
#include <private/LViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LSurfaceView.h>
#include <LLog.h>
#include <LFramebuffer.h>

using LVS = LView::LViewPrivate::LViewState;

void LSceneView::LSceneViewPrivate::calcNewDamage(LView *view)
{
    ThreadData *oD = currentThreadData;

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
            calcNewDamage(*it);
    }

    // Quick view cache handle to reduce verbosity
    LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

    view->imp()->removeFlag(LVS::RepaintCalled);

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

    bool colorFactorChanged = cache->voD->prevColorFactorEnabled != view->imp()->hasFlag(LVS::ColorFactor);

    if (!colorFactorChanged && view->imp()->hasFlag(LVS::ColorFactor))
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
            cache->voD->prevColorFactorEnabled = view->imp()->hasFlag(LVS::ColorFactor);
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

void LSceneView::LSceneViewPrivate::drawOpaqueDamage(LView *view)
{
    ThreadData *oD = currentThreadData;

    // Children first
    if (view->type() != Scene)
        for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
            drawOpaqueDamage(*it);

    LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

    if (!view->isRenderable() || !cache->mapped || cache->occluded || cache->opacity < 1.f || view->imp()->colorFactor.a < 1.f)
        return;

    cache->opaque.intersectRegion(oD->newDamage);
    cache->opaque.subtractRegion(cache->opaqueOverlay);

    oD->boxes = cache->opaque.boxes(&oD->n);

    if (view->imp()->hasFlag(LVS::ColorFactor))
    {
        oD->p->imp()->shaderSetColorFactor(view->imp()->colorFactor.r,
                             view->imp()->colorFactor.g,
                             view->imp()->colorFactor.b,
                             view->imp()->colorFactor.a);
    }
    else
        oD->p->imp()->shaderSetColorFactorEnabled(0);

    paintParams.p = oD->p;
    paintParams.alpha = 1.f;
    paintParams.scale = view->bufferScale();

    if (cache->scalingEnabled)
    {
        for (Int32 i = 0; i < oD->n; i++)
        {
            paintParams.dstX = oD->boxes->x1;
            paintParams.dstY = oD->boxes->y1;
            paintParams.dstW = oD->boxes->x2 - oD->boxes->x1;
            paintParams.dstH = oD->boxes->y2 - oD->boxes->y1;

            paintParams.srcX = (oD->boxes->x1 - cache->rect.x()) / cache->scalingVector.x();
            paintParams.srcY = (oD->boxes->y1  - cache->rect.y()) / cache->scalingVector.y();
            paintParams.srcW = paintParams.dstW / cache->scalingVector.x();
            paintParams.srcH = paintParams.dstH / cache->scalingVector.y();

            view->paintRect(paintParams);
            oD->boxes++;
        }
    }
    else
    {
        for (Int32 i = 0; i < oD->n; i++)
        {
            oD->w = oD->boxes->x2 - oD->boxes->x1;
            oD->h = oD->boxes->y2 - oD->boxes->y1;

            paintParams.dstX = oD->boxes->x1;
            paintParams.dstY = oD->boxes->y1;
            paintParams.dstW = oD->boxes->x2 - oD->boxes->x1;
            paintParams.dstH = oD->boxes->y2 - oD->boxes->y1;

            paintParams.srcX = oD->boxes->x1 - cache->rect.x();
            paintParams.srcY = oD->boxes->y1 - cache->rect.y();
            paintParams.srcW = paintParams.dstW;
            paintParams.srcH = paintParams.dstH;

            view->paintRect(paintParams);
            oD->boxes++;
        }
    }
}

void LSceneView::LSceneViewPrivate::drawBackground(bool addToOpaqueSum)
{
    ThreadData *oD = currentThreadData;
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

void LSceneView::LSceneViewPrivate::drawTranslucentDamage(LView *view)
{
    ThreadData *oD = currentThreadData;
    LView::LViewPrivate::ViewCache *cache = &view->imp()->cache;

    if (!view->isRenderable() || !cache->mapped || cache->occluded)
        goto drawChildrenOnly;

    if (view->autoBlendFuncEnabled())
    {
        if (oD->p->boundFramebuffer()->id() != 0)
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glBlendFuncSeparate(view->imp()->sRGBFactor, view->imp()->dRGBFactor, view->imp()->sAlphaFactor, view->imp()->dAlphaFactor);

    if (view->imp()->hasFlag(LVS::ColorFactor))
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
    paintParams.scale = view->bufferScale();
    paintParams.alpha = cache->opacity;

    if (cache->scalingEnabled)
    {
        for (Int32 i = 0; i < oD->n; i++)
        {
            paintParams.dstX = oD->boxes->x1;
            paintParams.dstY = oD->boxes->y1;
            paintParams.dstW = oD->boxes->x2 - oD->boxes->x1;
            paintParams.dstH = oD->boxes->y2 - oD->boxes->y1;

            paintParams.srcX = (oD->boxes->x1 - cache->rect.x()) / cache->scalingVector.x();
            paintParams.srcY = (oD->boxes->y1  - cache->rect.y()) / cache->scalingVector.y();
            paintParams.srcW = paintParams.dstW / cache->scalingVector.x();
            paintParams.srcH = paintParams.dstH / cache->scalingVector.y();

            view->paintRect(paintParams);
            oD->boxes++;
        }
    }
    else
    {
        for (Int32 i = 0; i < oD->n; i++)
        {
            paintParams.dstX = oD->boxes->x1;
            paintParams.dstY = oD->boxes->y1;
            paintParams.dstW = oD->boxes->x2 - oD->boxes->x1;
            paintParams.dstH = oD->boxes->y2 - oD->boxes->y1;

            paintParams.srcX = oD->boxes->x1 - cache->rect.x();
            paintParams.srcY = oD->boxes->y1 - cache->rect.y();
            paintParams.srcW = paintParams.dstW;
            paintParams.srcH = paintParams.dstH;

            view->paintRect(paintParams);
            oD->boxes++;
        }
    }

    drawChildrenOnly:
    if (view->type() != Scene)
        for (std::list<LView*>::const_iterator it = view->children().cbegin(); it != view->children().cend(); it++)
            drawTranslucentDamage(*it);
}

void LSceneView::LSceneViewPrivate::parentClipping(LView *parent, LRegion *region)
{
    if (!parent)
        return;

    region->clip(parent->pos(), parent->size());

    if (parent->parentClippingEnabled())
        parentClipping(parent->parent(), region);
}
