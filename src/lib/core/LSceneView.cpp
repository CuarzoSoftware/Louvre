#include <private/LSceneViewPrivate.h>
#include <LFramebuffer.h>
#include <LRenderBuffer.h>
#include <LPainter.h>
#include <LOutput.h>

LSceneView::LSceneView(LFramebuffer *framebuffer, LView *parent) : LView(Scene, parent)
{
    m_imp = new LSceneViewPrivate();
    imp()->fb = framebuffer;
}

const LRGBF &LSceneView::clearColor() const
{
    return imp()->clearColor;
}

void LSceneView::setClearColor(Float32 r, Float32 g, Float32 b)
{
    imp()->clearColor = {r, g, b};
}

void LSceneView::setClearColor(const LRGBF &color)
{
    imp()->clearColor = color;
}

void LSceneView::damageAll(LOutput *output)
{
    if (!output)
        return;

    LSceneViewPrivate::OutputData *oD = &imp()->outputsMap[output];

    if (oD->o)
    {
        LView *baseView = this;

        if (baseView->scene())
            oD->manuallyAddedDamage.addRect(oD->o->rectC());
        else
            oD->manuallyAddedDamage.addRect(imp()->fb->rectC());
    }
}

void LSceneView::addDamageC(LOutput *output, const LRegion &damage)
{
    if (!output)
        return;

    LSceneViewPrivate::OutputData *oD = &imp()->outputsMap[output];

    if (oD->o)
        oD->manuallyAddedDamage.addRegion(damage);
}

void LSceneView::render(LOutput *output)
{
    if (!output)
        return;

    output->painter()->bindFramebuffer(imp()->fb);

    LSceneViewPrivate::OutputData *oD = &imp()->outputsMap[output];

    // If output was not cached
    if (!oD->o)
    {
        for (Int32 i = 0; i < imp()->fb->buffersCount() - 1; i++)
            oD->prevDamageList.push_back(new LRegion());

        oD->c = compositor();
        oD->p = output->painter();
        oD->o = output;
    }

    imp()->clearTmpVariables(oD);
    imp()->checkOutputsScale(oD);
    imp()->checkRectChange(oD);

    // Add manual damage
    oD->newDamage.addRegion(oD->manuallyAddedDamage);
    oD->manuallyAddedDamage.clear();

    for (list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        imp()->calcNewDamage(*it, oD);

    // Save new damage for next frame and add old damage to current damage
    if (imp()->fb->buffersCount() > 1)
    {
        LRegion oldDamage = *oD->prevDamageList.front();

        for (std::list<LRegion*>::iterator it = std::next(oD->prevDamageList.begin()); it != oD->prevDamageList.end(); it++)
            oldDamage.addRegion(*(*it));

        LRegion *front = oD->prevDamageList.front();
        oD->prevDamageList.pop_front();
        oD->prevDamageList.push_back(front);

        *front = oD->newDamage;
        oD->newDamage.addRegion(oldDamage);
    }

    glDisable(GL_BLEND);

    for (list<LView*>::const_reverse_iterator it = children().crbegin(); it != children().crend(); it++)
        imp()->drawOpaqueDamage(*it, oD);

    imp()->drawBackground(oD);

    glEnable(GL_BLEND);

    for (list<LView*>::const_iterator it = children().cbegin(); it != children().cend(); it++)
        imp()->drawTranslucentDamage(*it, oD);

    imp()->fb->setFramebufferDamageC(&oD->newDamage);

    output->painter()->bindFramebuffer(output->framebuffer());
}

bool LSceneView::customPosEnabled() const
{
    return imp()->customPosEnabled;
}

void LSceneView::enableCustomPos(bool enabled)
{
    if (imp()->customPosEnabled != enabled)
    {
        imp()->customPosEnabled = enabled;
        repaint();
    }
}

void LSceneView::setCustomPosC(const LPoint &pos)
{
    if (customPosEnabled() && pos != imp()->customPos)
        repaint();

    imp()->customPos = pos;
}

const LPoint &LSceneView::customPosC() const
{
    return imp()->customPos;
}

bool LSceneView::nativeMapped() const
{
    return true;
}

const LPoint &LSceneView::nativePosC() const
{
    if (customPosEnabled())
        return imp()->customPos;

    return imp()->fb->rectC().pos();
}

const LSize &LSceneView::nativeSizeC() const
{
    return imp()->fb->rectC().size();
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

const LRegion *LSceneView::damageC() const
{
    return nullptr;//&imp()->damage;
}

const LRegion *LSceneView::translucentRegionC() const
{
    return nullptr;//&imp()->translucent;
}

const LRegion *LSceneView::opaqueRegionC() const
{
    return nullptr;//&imp()->opaque;
}

const LRegion *LSceneView::inputRegionC() const
{
    return &imp()->input;
}

void LSceneView::paintRectC(LPainter *p, Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH, Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH, Float32 scale, Float32 alpha)
{
    p->drawTextureC(imp()->fb->texture(imp()->fb->currentBufferIndex()),
                    srcX, srcY, srcW, srcH,
                    dstX, dstY, dstW, dstH,
                    scale, alpha);
}
