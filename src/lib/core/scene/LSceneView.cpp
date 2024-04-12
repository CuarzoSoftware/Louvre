#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <LSceneView.h>
#include <LUtils.h>

using namespace Louvre;

LSceneView::~LSceneView() noexcept
{
    // Need to remove children before LView destructor
    // or compositor crashes when children add damage
    while (!children().empty())
        children().front()->setParent(nullptr);

    if (!isLScene())
        delete m_fb;
}

void LSceneView::render(const LRegion *exclude) noexcept
{
    LPainter *painter { compositor()->imp()->threadsMap[std::this_thread::get_id()].painter };

    if (!painter)
        return;

    LFramebuffer *prevFb { painter->boundFramebuffer() };
    painter->bindFramebuffer(m_fb);

    if (!isLScene())
        static_cast<LRenderBuffer*>(m_fb)->setPos(pos());

    m_currentThreadData.reset(&m_sceneThreadsMap[std::this_thread::get_id()]);

    if (!m_currentThreadData)
        return;

    auto &ctd { *m_currentThreadData };

    // If painter was not cached
    if (!ctd.p)
    {
        for (Int32 i = 0; i < m_fb->buffersCount() - 1; i++)
            ctd.prevDamageList.push_back(new LRegion());

        ctd.p = painter;
        ctd.o = painter->imp()->output;
    }

    clearTmpVariables(ctd);
    checkRectChange(ctd);

    // Add manual damage
    if (!ctd.manuallyAddedDamage.empty())
    {
        ctd.newDamage.addRegion(ctd.manuallyAddedDamage);
        ctd.manuallyAddedDamage.clear();
    }

    // If extra opaque
    if (exclude)
    {
        ctd.prevExternalExclude.subtractRegion(*exclude);
        ctd.newDamage.addRegion(ctd.prevExternalExclude);
        ctd.prevExternalExclude = *exclude;
        ctd.opaqueSum.addRegion(*exclude);
    }
    else
    {
        if (!ctd.prevExternalExclude.empty())
        {
            ctd.newDamage.addRegion(ctd.prevExternalExclude);
            ctd.prevExternalExclude.clear();
        }
    }

    for (std::list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        calcNewDamage(*it);

    // Save new damage for next frame and add old damage to current damage
    if (m_fb->buffersCount() > 1)
    {
        // Sum damage generated in other frames into this region

        // This list contains NUM_FB - 1 regions, so if triple buffering then 2 regions (the prev ones to this current frame)
        for (std::list<LRegion*>::iterator it = std::next(ctd.prevDamageList.begin()); it != ctd.prevDamageList.end(); it++)
            (*ctd.prevDamageList.front()).addRegion(*(*it));

        pixman_region32_t tmp = ctd.prevDamageList.front()->m_region;
        ctd.prevDamageList.front()->m_region = ctd.newDamage.m_region;
        ctd.newDamage.m_region = tmp;
        ctd.newDamage.addRegion(*ctd.prevDamageList.front());

        LRegion *front = ctd.prevDamageList.front();
        ctd.prevDamageList.pop_front();
        ctd.prevDamageList.push_back(front);
    }

    glDisable(GL_BLEND);

    for (std::list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        drawOpaqueDamage(*it);

    painter->imp()->shaderSetColorFactorEnabled(0);
    drawBackground(!isLScene() && m_clearColor.a >= 1.f);

    glEnable(GL_BLEND);

    for (std::list<LView*>::const_iterator it = children().cbegin(); it != children().cend(); it++)
        drawTranslucentDamage(*it);

    if (!isLScene())
    {
        ctd.opaqueSum.clip(m_fb->rect());
        ctd.translucentSum = ctd.opaqueSum;
        ctd.translucentSum.inverse(m_fb->rect());
    }
    else
    {
        m_fb->setFramebufferDamage(&ctd.newDamage);
    }

    painter->bindFramebuffer(prevFb);
}

bool LSceneView::nativeMapped() const noexcept
{
    return true;
}

const LPoint &LSceneView::nativePos() const noexcept
{
    return m_customPos;
}

const LSize &LSceneView::nativeSize() const noexcept
{
    return m_fb->rect().size();
}

Float32 LSceneView::bufferScale() const noexcept
{
    return m_fb->scale();
}

void LSceneView::enteredOutput(LOutput *output) noexcept
{
    LVectorPushBackIfNonexistent(m_outputs, output);
}

void LSceneView::leftOutput(LOutput *output) noexcept
{
    LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput *> &LSceneView::outputs() const noexcept
{
    return m_outputs;
}

void LSceneView::requestNextFrame(LOutput *output) noexcept
{
    L_UNUSED(output);
}

const LRegion *LSceneView::damage() const noexcept
{
    if (m_currentThreadData)
        return &m_currentThreadData->newDamage;

    return nullptr;
}

const LRegion *LSceneView::translucentRegion() const noexcept
{
    if (m_currentThreadData)
        return &m_currentThreadData->translucentSum;

    return nullptr;
}

const LRegion *LSceneView::opaqueRegion() const noexcept
{
    if (m_currentThreadData)
        return &m_currentThreadData->opaqueSum;

    return nullptr;
}

const LRegion *LSceneView::inputRegion() const noexcept
{
    // TODO: add custom input region
    return nullptr;
}

void LSceneView::paintEvent(const PaintEventParams &params) noexcept
{
    params.painter->bindTextureMode({
        .texture = m_fb->texture(m_fb->currentBufferIndex()),
        .pos = pos(),
        .srcRect = LRectF(LPointF(), m_fb->sizeB()) / bufferScale(),
        .dstSize = size(),
        .srcTransform = m_fb->transform(),
        .srcScale = bufferScale(),
    });

    params.painter->enableCustomTextureColor(false);
    params.painter->drawRegion(*params.region);
}

void LSceneView::calcNewDamage(LView *view) noexcept
{
    auto &ctd { *m_currentThreadData };

    // Children first
    if (view->type() == Scene)
    {
        LSceneView &sceneView { static_cast<LSceneView&>(*view) };

        if (view->m_cache.scalingEnabled)
            sceneView.render(nullptr);
        else
            sceneView.render(&ctd.opaqueSum);
    }
    else
    {
        for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
            calcNewDamage(*it);
    }

    // Quick view cache handle to reduce verbosity
    LView::ViewCache &cache { view->m_cache };

    view->m_state.remove(RepaintCalled);

    cache.voD = &view->m_threadsMap[std::this_thread::get_id()];
    cache.voD->o = ctd.o;
    cache.mapped = view->mapped();
    cache.rect.setPos(view->pos());
    cache.rect.setSize(view->size());
    cache.scalingVector = view->scalingVector();
    cache.scalingEnabled = (view->scalingEnabled() || view->parentScalingEnabled()) && cache.scalingVector != LSizeF(1.f, 1.f);

    LRegion vRegion {cache.rect};

    if (view->clippingEnabled())
        vRegion.clip(view->clippingRect());

    if (view->parent() && view->parentClippingEnabled())
        vRegion.clip(view->parent()->pos(), view->parent()->size());

    // Update view intersected outputs
    for (LOutput *o : compositor()->outputs())
    {
        LRegion r { vRegion };
        r.clip(o->rect());

        if (!r.empty())
            view->enteredOutput(o);
        else
            view->leftOutput(o);
    }

    if (!view->isRenderable())
        return;

    cache.opacity = view->opacity();

    if (view->m_colorFactor.a <= 0.f || cache.rect.size().area() == 0 || cache.opacity <= 0.f || cache.scalingVector.w() == 0.f || cache.scalingVector.y() == 0.f || (view->clippingEnabled() && view->clippingRect().area() == 0))
        cache.mapped = false;

    const bool mappingChanged { cache.mapped != cache.voD->prevMapped };

    if (ctd.o && !mappingChanged && !cache.mapped)
    {
        if (view->forceRequestNextFrameEnabled())
            view->requestNextFrame(ctd.o);
        return;
    }

    const bool opacityChanged { cache.opacity != cache.voD->prevOpacity };

    cache.localRect = LRect(cache.rect.pos() - m_fb->rect().pos(), cache.rect.size());

    const bool rectChanged { cache.localRect != cache.voD->prevLocalRect };

    bool colorFactorChanged { cache.voD->prevColorFactorEnabled != view->m_state.check(ColorFactor) };

    if (!colorFactorChanged && view->m_state.check(ColorFactor))
    {
        colorFactorChanged = cache.voD->prevColorFactor.r != view->m_colorFactor.r ||
                             cache.voD->prevColorFactor.g != view->m_colorFactor.g ||
                             cache.voD->prevColorFactor.b != view->m_colorFactor.b ||
                             cache.voD->prevColorFactor.a != view->m_colorFactor.a;
    }

    // If rect or order changed (set current rect and prev rect as damage)
    if (mappingChanged || rectChanged || cache.voD->changedOrder || opacityChanged || cache.scalingEnabled || colorFactorChanged)
    {
        cache.damage.addRect(cache.rect);

        if (cache.voD->changedOrder)
            cache.voD->changedOrder = false;

        if (mappingChanged)
            cache.voD->prevMapped = cache.mapped;

        if (rectChanged)
        {
            cache.voD->prevRect = cache.rect;
            cache.voD->prevLocalRect = cache.localRect;
        }

        if (opacityChanged)
            cache.voD->prevOpacity = cache.opacity;

        if (colorFactorChanged)
        {
            cache.voD->prevColorFactorEnabled = view->m_state.check(ColorFactor);
            cache.voD->prevColorFactor = view->m_colorFactor;
        }

        if (!cache.mapped)
        {
            ctd.newDamage.addRegion(cache.voD->prevClipping);
            return;
        }
    }
    else if (view->damage())
    {
        cache.damage = *view->damage();

        // Scene views already have their damage transposed
        if (view->type() != Scene)
            cache.damage.offset(cache.rect.pos());
    }
    else
    {
        cache.damage.clear();
    }

    // Calculates the non clipped region

    LRegion currentClipping { cache.rect };

    if (view->parentClippingEnabled())
        parentClipping(view->parent(), &currentClipping);

    if (view->clippingEnabled())
        currentClipping.clip(view->clippingRect());

    // Calculates the new exposed view region if parent clipping or clipped region has grown

    /* LRegion newExposedClipping = currentClipping;
     * newExposedClipping.subtractRegion(cache.voD->prevClipping);*/

    LRegion newExposedClipping;
    pixman_region32_subtract(&newExposedClipping.m_region,
                             &currentClipping.m_region,
                             &cache.voD->prevClipping.m_region);

    cache.damage.addRegion(newExposedClipping);

    // Add exposed now non clipped region to new output damage
    cache.voD->prevClipping.subtractRegion(currentClipping);
    ctd.newDamage.addRegion(cache.voD->prevClipping);

    // Saves current clipped region for next frame
    cache.voD->prevClipping = currentClipping;

    // Clip current damage to current visible region
    cache.damage.intersectRegion(currentClipping);

    // Remove previus opaque region to view damage
    cache.damage.subtractRegion(ctd.opaqueSum);

    // Add clipped damage to new damage
    ctd.newDamage.addRegion(cache.damage);

    if (cache.opacity < 1.f || cache.scalingEnabled || view->colorFactor().a < 1.f)
    {
        cache.translucent.clear();
        cache.translucent.addRect(cache.rect);
        cache.opaque.clear();
    }
    else
    {
        // Store tansposed traslucent region
        if (view->translucentRegion())
        {
            cache.translucent = *view->translucentRegion();

            if (view->type() != Scene)
                cache.translucent.offset(cache.rect.pos());
        }
        else
        {
            cache.translucent.clear();
            cache.translucent.addRect(cache.rect);
        }

        // Store tansposed opaque region
        if (view->opaqueRegion())
        {
            cache.opaque = *view->opaqueRegion();

            if (view->type() != Scene)
                cache.opaque.offset(cache.rect.pos());
        }
        else
        {
            cache.opaque = cache.translucent;
            cache.opaque.inverse(cache.rect);
        }
    }

    // Clip opaque and translucent regions to current visible region
    cache.opaque.intersectRegion(currentClipping);
    cache.translucent.intersectRegion(currentClipping);

    // Check if view is ocludded
    currentClipping.subtractRegion(ctd.opaqueSum);

    cache.occluded = currentClipping.empty();

    if (ctd.o && (!cache.occluded || view->forceRequestNextFrameEnabled()))
        view->requestNextFrame(ctd.o);

    // Store sum of previus opaque regions (this will later be clipped when painting opaque and translucent regions)
    cache.opaqueOverlay = ctd.opaqueSum;
    ctd.opaqueSum.addRegion(cache.opaque);
}

void LSceneView::drawOpaqueDamage(LView *view) noexcept
{
    auto &ctd { *m_currentThreadData };

    // Children first
    if (view->type() != Scene)
        for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
            drawOpaqueDamage(*it);

    LView::ViewCache &cache { view->m_cache };

    if (!view->isRenderable() || !cache.mapped || cache.occluded || cache.opacity < 1.f || view->m_colorFactor.a < 1.f)
        return;

    cache.opaque.intersectRegion(ctd.newDamage);
    cache.opaque.subtractRegion(cache.opaqueOverlay);

    if (view->m_state.check(ColorFactor))
    {
        ctd.p->imp()->shaderSetColorFactor(view->m_colorFactor.r,
                                          view->m_colorFactor.g,
                                          view->m_colorFactor.b,
                                          view->m_colorFactor.a);
    }
    else
        ctd.p->imp()->shaderSetColorFactorEnabled(0);

    ctd.p->imp()->shaderSetAlpha(1.f);
    m_paintParams.painter = ctd.p;
    m_paintParams.region = &cache.opaque;
    view->paintEvent(m_paintParams);
}

void LSceneView::drawTranslucentDamage(LView *view) noexcept
{
    auto &ctd { *m_currentThreadData };
    auto &cache { view->m_cache };

    if (!view->isRenderable() || !cache.mapped || cache.occluded)
        goto drawChildrenOnly;

    if (view->autoBlendFuncEnabled())
    {
        /*if (ctd.p->boundFramebuffer()->id() != 0)
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        else*/

        if (view->type() == LView::Type::Surface)
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        else
          glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    }
    else
        glBlendFuncSeparate(view->blendFunc().sRGBFactor,
                            view->blendFunc().dRGBFactor,
                            view->blendFunc().sAlphaFactor,
                            view->blendFunc().dAlphaFactor);

    if (view->m_state.check(ColorFactor))
    {
        ctd.p->imp()->shaderSetColorFactor(view->m_colorFactor.r,
                                          view->m_colorFactor.g,
                                          view->m_colorFactor.b,
                                          view->m_colorFactor.a);
    }
    else
        ctd.p->imp()->shaderSetColorFactorEnabled(0);

    cache.occluded = true;
    cache.translucent.intersectRegion(ctd.newDamage);
    cache.translucent.subtractRegion(cache.opaqueOverlay);

    ctd.p->imp()->shaderSetAlpha(cache.opacity);
    m_paintParams.painter = ctd.p;
    m_paintParams.region = &cache.translucent;
    view->paintEvent(m_paintParams);

drawChildrenOnly:
    if (view->type() != Scene)
        for (std::list<LView*>::const_iterator it = view->children().cbegin(); it != view->children().cend(); it++)
            drawTranslucentDamage(*it);
}


