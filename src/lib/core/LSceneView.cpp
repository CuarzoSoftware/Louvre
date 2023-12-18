#include <private/LCompositorPrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <LFramebuffer.h>
#include <LRenderBuffer.h>
#include <LOutput.h>

LSceneView::LSceneView(LFramebuffer *framebuffer, LView *parent) :
    LView(Scene, parent),
    LPRIVATE_INIT_UNIQUE(LSceneView)
{
    imp()->fb = framebuffer;
}

LSceneView::LSceneView(const LSize &sizeB, Int32 bufferScale, LView *parent) :
    LView(Scene, parent),
    LPRIVATE_INIT_UNIQUE(LSceneView)
{
    imp()->fb = new LRenderBuffer(sizeB);
    LRenderBuffer *rb = (LRenderBuffer*)imp()->fb;
    rb->setScale(bufferScale);
}

LSceneView::~LSceneView()
{
    // Need to remove children before LView destructor
    // or compositor crashes when children add damage
    while (!children().empty())
        children().front()->setParent(nullptr);

    if (!isLScene())
        delete imp()->fb;
}

const LRGBAF &LSceneView::clearColor() const
{
    return imp()->clearColor;
}

void LSceneView::setClearColor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    imp()->clearColor = {r, g, b, a};
    repaint();
}

void LSceneView::setClearColor(const LRGBAF &color)
{
    imp()->clearColor = color;
    repaint();
}

void LSceneView::damageAll(LOutput *output)
{
    if (!output)
        return;

    LSceneViewPrivate::ThreadData *oD = &imp()->threadsMap[output->threadId()];

    if (isLScene())
        oD->manuallyAddedDamage.addRect(output->rect());
    else
        oD->manuallyAddedDamage.addRect(LRect(pos(), size()));

}

void LSceneView::addDamage(LOutput *output, const LRegion &damage)
{
    if (!output)
        return;

    LSceneViewPrivate::ThreadData *oD = &imp()->threadsMap[output->threadId()];

    if (oD->o)
        oD->manuallyAddedDamage.addRegion(damage);
}

bool LSceneView::isLScene() const
{
    LView *nativeView = (LSceneView*)this;
    return nativeView->imp()->scene != nullptr;
}

void LSceneView::render(const LRegion *exclude)
{
    LPainter *painter = compositor()->imp()->threadsMap[std::this_thread::get_id()].painter;

    if (!painter)
        return;

    LFramebuffer *prevFb = painter->boundFramebuffer();

    painter->bindFramebuffer(imp()->fb);

    if (!isLScene())
    {
        LRenderBuffer *rb = (LRenderBuffer*)imp()->fb;
        rb->setPos(pos());
    }

    LSceneViewPrivate::ThreadData *oD = &imp()->threadsMap[std::this_thread::get_id()];
    imp()->currentThreadData = oD;

    // If painter was not cached
    if (!oD->p)
    {
        for (Int32 i = 0; i < imp()->fb->buffersCount() - 1; i++)
            oD->prevDamageList.push_back(new LRegion());

        oD->c = compositor();
        oD->p = painter;
        oD->o = painter->imp()->output;
    }

    imp()->clearTmpVariables(oD);
    imp()->checkRectChange(oD);

    // Add manual damage
    if (!oD->manuallyAddedDamage.empty())
    {
        oD->newDamage.addRegion(oD->manuallyAddedDamage);
        oD->manuallyAddedDamage.clear();
    }

    // If extra opaque
    if (exclude)
    {
        oD->prevExternalExclude.subtractRegion(*exclude);
        oD->newDamage.addRegion(oD->prevExternalExclude);
        oD->prevExternalExclude = *exclude;
        oD->opaqueTransposedSum.addRegion(*exclude);
    }
    else
    {
        if (!oD->prevExternalExclude.empty())
        {
            oD->newDamage.addRegion(oD->prevExternalExclude);
            oD->prevExternalExclude.clear();
        }
    }

    for (std::list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        imp()->calcNewDamage(*it);

    // Save new damage for next frame and add old damage to current damage
    if (imp()->fb->buffersCount() > 1)
    {
        // Sum damage generated in other frames into this region

        // This list contains NUM_FB - 1 regions, so if triple buffering then 2 regions (the prev ones to this current frame)
        for (std::list<LRegion*>::iterator it = std::next(oD->prevDamageList.begin()); it != oD->prevDamageList.end(); it++)
            (*oD->prevDamageList.front()).addRegion(*(*it));

        pixman_region32_t tmp = oD->prevDamageList.front()->m_region;
        oD->prevDamageList.front()->m_region = oD->newDamage.m_region;
        oD->newDamage.m_region = tmp;
        oD->newDamage.addRegion(*oD->prevDamageList.front());

        LRegion *front = oD->prevDamageList.front();
        oD->prevDamageList.pop_front();
        oD->prevDamageList.push_back(front);
    }

    glDisable(GL_BLEND);

    for (std::list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        imp()->drawOpaqueDamage(*it);

    painter->imp()->shaderSetColorFactorEnabled(0);
    imp()->drawBackground(!isLScene() && imp()->clearColor.a >= 1.f);

    glEnable(GL_BLEND);

    for (std::list<LView*>::const_iterator it = children().cbegin(); it != children().cend(); it++)
        imp()->drawTranslucentDamage(*it);

    if (!isLScene())
    {
        oD->opaqueTransposedSum.clip(imp()->fb->rect());
        oD->translucentTransposedSum = oD->opaqueTransposedSum;
        oD->translucentTransposedSum.inverse(imp()->fb->rect());
    }
    else
    {
        imp()->fb->setFramebufferDamage(&oD->newDamage);
    }

    painter->bindFramebuffer(prevFb);
}

LTexture *LSceneView::texture(Int32 index) const
{
    return (LTexture*)imp()->fb->texture(index);
}

void LSceneView::setPos(const LPoint &pos)
{
    setPos(pos.x(), pos.y());
}

void LSceneView::setPos(Int32 x, Int32 y)
{
    if (x != imp()->customPos.x() || y != imp()->customPos.y())
    {
        imp()->customPos.setX(x);
        imp()->customPos.setY(y);

        if (!isLScene())
        {
            LRenderBuffer *rb = (LRenderBuffer*)imp()->fb;
            rb->setPos(imp()->customPos);
        }
        repaint();
    }
}

void LSceneView::setSizeB(const LSize &size)
{
    if (!isLScene() && size != imp()->fb->sizeB())
    {
        LRenderBuffer *rb = (LRenderBuffer*)imp()->fb;
        rb->setSizeB(size);
        for (LOutput *o : compositor()->outputs())
            damageAll(o);
        repaint();
    }
}

void LSceneView::setScale(Int32 scale)
{
    if (!isLScene() && bufferScale() != scale)
    {
        LRenderBuffer *rb = (LRenderBuffer*)imp()->fb;
        rb->setScale(scale);
        for (LOutput *o : compositor()->outputs())
            damageAll(o);
        repaint();
    }
}

bool LSceneView::nativeMapped() const
{
    return true;
}

const LPoint &LSceneView::nativePos() const
{
    return imp()->customPos;
}

const LSize &LSceneView::nativeSize() const
{
    return imp()->fb->rect().size();
}

Int32 LSceneView::bufferScale() const
{
    return imp()->fb->scale();
}

void LSceneView::enteredOutput(LOutput *output)
{
    imp()->outputs.remove(output);
    imp()->outputs.push_back(output);
}

void LSceneView::leftOutput(LOutput *output)
{
    imp()->outputs.remove(output);
}

const std::list<LOutput *> &LSceneView::outputs() const
{
    return imp()->outputs;
}

bool LSceneView::isRenderable() const
{
    return true;
}

void LSceneView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LSceneView::damage() const
{
    return &imp()->currentThreadData->newDamage;
}

const LRegion *LSceneView::translucentRegion() const
{
    return &imp()->currentThreadData->translucentTransposedSum;
}

const LRegion *LSceneView::opaqueRegion() const
{
    return &imp()->currentThreadData->opaqueTransposedSum;
}

const LRegion *LSceneView::inputRegion() const
{
    return &imp()->input;
}

void LSceneView::paintRect(const PaintRectParams &params)
{
    params.p->imp()->drawTexture(imp()->fb->texture(imp()->fb->currentBufferIndex()),
                                 params.srcX, params.srcY, params.srcW, params.srcH,
                                 params.dstX, params.dstY, params.dstW, params.dstH,
                                 params.scale, params.alpha);
}
