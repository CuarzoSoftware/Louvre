#include <LScene.h>
#include <LSceneView.h>
#include <LSurfaceView.h>
#include <LUtils.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>

using namespace Louvre;

LSceneView::~LSceneView() noexcept {
  notifyDestruction();

  // Need to remove children before LView destructor
  // or compositor crashes when children add damage
  while (!children().empty()) children().front()->setParent(nullptr);

  if (!isLScene()) delete m_fb;
}

void LSceneView::damageAll(LOutput *output) noexcept {
  if (!output) return;

  ThreadData &td{m_sceneThreadsMap[output->threadId()]};

  if (isLScene())
    td.manuallyAddedDamage.addRect(output->rect());
  else {
    td.manuallyAddedDamage.addRect(LRect(pos(), size()));

    if (scene() && scene()->autoRepaintEnabled()) output->repaint();
  }
}

void LSceneView::addDamage(LOutput *output, const LRegion &damage) noexcept {
  if (!output) return;

  ThreadData &td{m_sceneThreadsMap[output->threadId()]};

  if (td.o) td.manuallyAddedDamage.addRegion(damage);

  if (!isLScene() && scene() && scene()->autoRepaintEnabled())
    output->repaint();
}

void LSceneView::render(const LRegion *exclude) noexcept {
  LPainter *painter{compositor()->imp()->findPainter()};

  if (!painter) return;

  LFramebuffer *prevFb{painter->boundFramebuffer()};
  painter->bindFramebuffer(m_fb);

  if (!isLScene()) static_cast<LRenderBuffer *>(m_fb)->setPos(pos());

  m_currentThreadData.reset(&m_sceneThreadsMap[std::this_thread::get_id()]);

  if (!m_currentThreadData) return;

  auto &ctd{*m_currentThreadData};

  // If painter was not cached
  if (!ctd.p) {
    ctd.p = painter;
    ctd.o = painter->imp()->output;
  }

  clearTmpVariables(ctd);
  checkRectChange(ctd);

  // Add manual damage
  if (!ctd.manuallyAddedDamage.empty()) {
    ctd.newDamage.addRegion(ctd.manuallyAddedDamage);
    ctd.manuallyAddedDamage.clear();
  }

  // If extra opaque
  if (exclude) {
    ctd.prevExternalExclude.subtractRegion(*exclude);
    ctd.newDamage.addRegion(ctd.prevExternalExclude);
    ctd.prevExternalExclude = *exclude;
    ctd.opaqueSum.addRegion(*exclude);
  } else {
    if (!ctd.prevExternalExclude.empty()) {
      ctd.newDamage.addRegion(ctd.prevExternalExclude);
      ctd.prevExternalExclude.clear();
    }
  }

  for (std::list<LView *>::const_reverse_iterator it = children().crbegin();
       it != children().crend(); it++)
    calcNewDamage(*it);

  Int32 age{m_fb->bufferAge()};

  if (age > LSCENE_MAX_AGE) age = 0;

  if (age == 0) {
    ctd.newDamage.addRect(m_fb->rect());
    ctd.damageRing[ctd.damageRingIndex] = ctd.newDamage;
  } else {
    ctd.damageRing[ctd.damageRingIndex] = ctd.newDamage;

    for (Int32 i = 1; i < age; i++) {
      Int32 damageIndex = ctd.damageRingIndex - i;

      if (damageIndex < 0) damageIndex = LSCENE_MAX_AGE + damageIndex;

      ctd.newDamage.addRegion(ctd.damageRing[damageIndex]);
    }
  }

  if (ctd.damageRingIndex == LSCENE_MAX_AGE - 1)
    ctd.damageRingIndex = 0;
  else
    ctd.damageRingIndex++;

  glDisable(GL_BLEND);

  for (std::list<LView *>::const_reverse_iterator it = children().crbegin();
       it != children().crend(); it++)
    drawOpaqueDamage(*it);

  drawBackground(!isLScene() && m_clearColor.a >= 1.f);

  glEnable(GL_BLEND);

  for (std::list<LView *>::const_iterator it = children().cbegin();
       it != children().cend(); it++)
    drawTranslucentDamage(*it);

  if (!isLScene()) {
    ctd.opaqueSum.clip(m_fb->rect());
    ctd.translucentSum = ctd.opaqueSum;
    ctd.translucentSum.inverse(m_fb->rect());
    auto &rb = *static_cast<LRenderBuffer *>(m_fb);
    rb.setFence();
  } else {
    m_fb->setFramebufferDamage(&ctd.newDamage);
  }

  painter->bindFramebuffer(prevFb);
}

bool LSceneView::nativeMapped() const noexcept { return true; }

const LPoint &LSceneView::nativePos() const noexcept { return m_customPos; }

const LSize &LSceneView::nativeSize() const noexcept {
  return m_fb->rect().size();
}

Float32 LSceneView::bufferScale() const noexcept { return m_fb->scale(); }

void LSceneView::enteredOutput(LOutput *output) noexcept {
  LVectorPushBackIfNonexistent(m_outputs, output);
}

void LSceneView::leftOutput(LOutput *output) noexcept {
  LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput *> &LSceneView::outputs() const noexcept {
  return m_outputs;
}

void LSceneView::requestNextFrame(LOutput *output) noexcept {
  L_UNUSED(output);
}

const LRegion *LSceneView::damage() const noexcept {
  if (m_currentThreadData) return &m_currentThreadData->newDamage;

  return nullptr;
}

const LRegion *LSceneView::translucentRegion() const noexcept {
  if (m_currentThreadData) return &m_currentThreadData->translucentSum;

  return nullptr;
}

const LRegion *LSceneView::opaqueRegion() const noexcept {
  if (m_currentThreadData) return &m_currentThreadData->opaqueSum;

  return nullptr;
}

const LRegion *LSceneView::inputRegion() const noexcept {
  // TODO: add option for custom input regions
  return nullptr;
}

void LSceneView::paintEvent(const PaintEventParams &params) noexcept {
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

void LSceneView::calcNewDamage(LView *view) noexcept {
  auto &ctd{*m_currentThreadData};

  // Children first
  if (view->type() == SceneType) {
    LSceneView &sceneView{static_cast<LSceneView &>(*view)};

    if (view->m_cache.scalingEnabled)
      sceneView.render(nullptr);
    else
      sceneView.render(&ctd.opaqueSum);
  } else {
    for (std::list<LView *>::const_reverse_iterator it =
             view->children().crbegin();
         it != view->children().crend(); it++)
      calcNewDamage(*it);
  }

  // Quick view cache handle to reduce verbosity
  LView::ViewCache &cache{view->m_cache};

  view->m_state.remove(RepaintCalled);

  cache.voD = &view->m_threadsMap[std::this_thread::get_id()];
  cache.voD->o = ctd.o;
  cache.mapped = view->mapped();
  cache.rect.setPos(view->pos());
  cache.rect.setSize(view->size());
  cache.scalingVector = view->scalingVector();
  cache.scalingEnabled =
      (view->scalingEnabled() || view->parentScalingEnabled()) &&
      cache.scalingVector != LSizeF(1.f, 1.f);

  LRegion vRegion{cache.rect};

  if (view->clippingEnabled()) vRegion.clip(view->clippingRect());

  if (view->parent() && view->parentClippingEnabled())
    vRegion.clip(view->parent()->pos(), view->parent()->size());

  // Update view intersected outputs
  for (LOutput *o : compositor()->outputs()) {
    LRegion r{vRegion};
    r.clip(o->rect());

    if (!r.empty())
      view->enteredOutput(o);
    else
      view->leftOutput(o);
  }

  /*
  // TODO add api
  if (view->type() == LView::Type::Surface)
  {
      LSurface *surface { static_cast<LSurfaceView*>(view)->surface() };

      if (surface && surface->role() && surface->role()->exclusiveOutput())
      {
          view->enteredOutput(surface->role()->exclusiveOutput());
          surface->requestNextFrame(false);
      }
  }
  */

  if (!view->isRenderable()) return;

  cache.opacity = view->opacity();

  if (view->m_colorFactor.a <= 0.f || cache.rect.size().area() == 0 ||
      cache.opacity <= 0.f || cache.scalingVector.w() == 0.f ||
      cache.scalingVector.y() == 0.f ||
      (view->clippingEnabled() && view->clippingRect().area() == 0))
    cache.mapped = false;

  const bool mappingChanged{cache.mapped != cache.voD->prevMapped};

  if (ctd.o && !mappingChanged && !cache.mapped) {
    if (view->forceRequestNextFrameEnabled()) view->requestNextFrame(ctd.o);
    return;
  }

  const bool opacityChanged{cache.opacity != cache.voD->prevOpacity};

  cache.localRect =
      LRect(cache.rect.pos() - m_fb->rect().pos(), cache.rect.size());

  const bool rectChanged{cache.localRect != cache.voD->prevLocalRect};

  bool colorFactorChanged{cache.voD->prevColorFactorEnabled !=
                          view->m_state.check(ColorFactor)};

  if (!colorFactorChanged && view->m_state.check(ColorFactor)) {
    colorFactorChanged =
        cache.voD->prevColorFactor.r != view->m_colorFactor.r ||
        cache.voD->prevColorFactor.g != view->m_colorFactor.g ||
        cache.voD->prevColorFactor.b != view->m_colorFactor.b ||
        cache.voD->prevColorFactor.a != view->m_colorFactor.a;
  }

  // If rect or order changed (set current rect and prev rect as damage)
  if (mappingChanged || rectChanged || cache.voD->changedOrder ||
      opacityChanged || cache.scalingEnabled || colorFactorChanged) {
    cache.damage.addRect(cache.rect);

    if (cache.voD->changedOrder) cache.voD->changedOrder = false;

    if (mappingChanged) cache.voD->prevMapped = cache.mapped;

    if (rectChanged) {
      cache.voD->prevRect = cache.rect;
      cache.voD->prevLocalRect = cache.localRect;
    }

    if (opacityChanged) cache.voD->prevOpacity = cache.opacity;

    if (colorFactorChanged) {
      cache.voD->prevColorFactorEnabled = view->m_state.check(ColorFactor);
      cache.voD->prevColorFactor = view->m_colorFactor;
    }

    if (!cache.mapped) {
      ctd.newDamage.addRegion(cache.voD->prevClipping);
      return;
    }
  } else if (view->damage()) {
    cache.damage = *view->damage();

    // Scene views already have their damage transposed
    if (view->type() != SceneType) cache.damage.offset(cache.rect.pos());
  } else {
    cache.damage.clear();
  }

  // Calculates the non clipped region

  LRegion currentClipping{cache.rect};

  if (view->parentClippingEnabled())
    parentClipping(view->parent(), &currentClipping);

  if (view->clippingEnabled()) currentClipping.clip(view->clippingRect());

  // Calculates the new exposed view region if parent clipping or clipped region
  // has grown

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

  if (cache.opacity < 1.f || cache.scalingEnabled ||
      view->colorFactor().a < 1.f) {
    cache.translucent.clear();
    cache.translucent.addRect(cache.rect);
    cache.opaque.clear();
  } else {
    // Store tansposed traslucent region
    if (view->translucentRegion()) {
      cache.translucent = *view->translucentRegion();

      if (view->type() != SceneType) cache.translucent.offset(cache.rect.pos());
    } else {
      cache.translucent.clear();
      cache.translucent.addRect(cache.rect);
    }

    // Store tansposed opaque region
    if (view->opaqueRegion()) {
      cache.opaque = *view->opaqueRegion();

      if (view->type() != SceneType) cache.opaque.offset(cache.rect.pos());
    } else {
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

  // Store sum of previus opaque regions (this will later be clipped when
  // painting opaque and translucent regions)
  cache.opaqueOverlay = ctd.opaqueSum;
  ctd.opaqueSum.addRegion(cache.opaque);
}

void LSceneView::drawOpaqueDamage(LView *view) noexcept {
  auto &ctd{*m_currentThreadData};

  // Children first
  if (view->type() != SceneType)
    for (std::list<LView *>::const_reverse_iterator it =
             view->children().crbegin();
         it != view->children().crend(); it++)
      drawOpaqueDamage(*it);

  LView::ViewCache &cache{view->m_cache};

  if (!view->isRenderable() || !cache.mapped || cache.occluded ||
      cache.opacity < 1.f || view->m_colorFactor.a < 1.f)
    return;

  cache.opaque.intersectRegion(ctd.newDamage);
  cache.opaque.subtractRegion(cache.opaqueOverlay);

  ctd.p->enableAutoBlendFunc(view->autoBlendFuncEnabled());

  if (view->m_state.check(ColorFactor))
    ctd.p->setColorFactor(view->m_colorFactor);
  else
    ctd.p->setColorFactor(1.f, 1.f, 1.f, 1.f);

  ctd.p->setAlpha(1.f);
  m_paintParams.painter = ctd.p;
  m_paintParams.region = &cache.opaque;
  view->paintEvent(m_paintParams);
}

void LSceneView::drawTranslucentDamage(LView *view) noexcept {
  auto &ctd{*m_currentThreadData};
  auto &cache{view->m_cache};

  if (!view->isRenderable() || !cache.mapped || cache.occluded)
    goto drawChildrenOnly;

  ctd.p->enableAutoBlendFunc(view->autoBlendFuncEnabled());

  if (!view->autoBlendFuncEnabled()) ctd.p->setBlendFunc(view->blendFunc());

  if (view->m_state.check(ColorFactor))
    ctd.p->setColorFactor(view->m_colorFactor);
  else
    ctd.p->setColorFactor(1.f, 1.f, 1.f, 1.f);

  cache.occluded = true;
  cache.translucent.intersectRegion(ctd.newDamage);
  cache.translucent.subtractRegion(cache.opaqueOverlay);

  ctd.p->setAlpha(cache.opacity);
  m_paintParams.painter = ctd.p;
  m_paintParams.region = &cache.translucent;
  view->paintEvent(m_paintParams);

drawChildrenOnly:
  if (view->type() != SceneType)
    for (std::list<LView *>::const_iterator it = view->children().cbegin();
         it != view->children().cend(); it++)
      drawTranslucentDamage(*it);
}
