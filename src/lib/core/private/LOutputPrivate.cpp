#include <protocols/Wayland/private/GOutputPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSeat.h>
#include <LClient.h>

#include <LTime.h>
#include <iostream>

LOutput::LOutputPrivate::LOutputPrivate(LOutput *output) : fb(output) {}

// This is called from LCompositor::addOutput()
bool LOutput::LOutputPrivate::initialize()
{
    output->imp()->state = LOutput::PendingInitialize;
    return compositor()->imp()->graphicBackend->outputInitialize(output);
}

void LOutput::LOutputPrivate::backendInitializeGL()
{
    if (output->gammaSize() != 0)
        output->setGamma(nullptr);

    threadId = std::this_thread::get_id();

    painter = new LPainter();
    painter->imp()->output = output;
    painter->bindFramebuffer(output->framebuffer());

    output->imp()->global = wl_global_create(compositor()->display(),
                                             &wl_output_interface,
                                             LOUVRE_WL_OUTPUT_VERSION,
                                             output,
                                             &Protocols::Wayland::GOutput::GOutputPrivate::bind);

    output->setScale(output->imp()->fractionalScale);
    lastPos = rect.pos();
    lastSize = rect.size();
    cursor()->imp()->textureChanged = true;
    cursor()->imp()->update();
    output->imp()->state = LOutput::Initialized;
    output->initializeGL();
    compositor()->flushClients();
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state != LOutput::Initialized)
        return;

    if (callLock)
        compositor()->imp()->lock();

    if (compositor()->imp()->runningAnimations() && seat()->enabled())
        compositor()->imp()->unlockPoll();

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }

    if (lastSize != rect.size())
    {
        output->resizeGL();
        lastSize = rect.size();
    }

    compositor()->imp()->sendPresentationTime();
    compositor()->imp()->processAnimations();
    stateFlags.remove(PendingRepaint);
    painter->bindFramebuffer(&fb);

    compositor()->imp()->currentOutput = output;
    output->paintGL();
    compositor()->imp()->currentOutput = nullptr;

    if (stateFlags.check(HasDamage) && (stateFlags.checkAll(UsingFractionalScale | FractionalOversamplingEnabled) || output->hasBufferDamageSupport()))
    {
        damage.offset(-rect.pos().x(), -rect.pos().y());
        damage.transform(rect.size(), transform);

        pixman_region32_t tmp;
        pixman_region32_init(&tmp);

        Int32 n;
        pixman_box32_t *rects = pixman_region32_rectangles(&damage.m_region, &n);

        for (Int32 i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                &tmp,
                &tmp,
                floorf(Float32(rects->x1) * fractionalScale) - 2,
                floorf(Float32(rects->y1) * fractionalScale) - 2,
                ceilf(Float32(rects->x2 - rects->x1) * fractionalScale) + 4,
                ceilf(Float32(rects->y2 - rects->y1) * fractionalScale) + 4);
            rects++;
        }

        pixman_region32_fini(&damage.m_region);
        damage.m_region = tmp;

        damage.clip(LRect(0, output->currentMode()->sizeB()));

        if (output->hasBufferDamageSupport())
            compositor()->imp()->graphicBackend->outputSetBufferDamage(output, damage);
    }

    if (stateFlags.checkAll(UsingFractionalScale | FractionalOversamplingEnabled))
    {
        stateFlags.remove(UsingFractionalScale);
        LFramebuffer::Transform prevTrasform = transform;
        transform = LFramebuffer::Normal;
        Float32 prevScale = scale;
        scale = 1.f;
        LPoint prevPos = rect.pos();
        rect.setPos(LPoint(0));
        updateRect();
        painter->bindFramebuffer(&fb);
        painter->enableCustomTextureColor(false);
        painter->bindTextureMode({
            .texture = fractionalFb.texture(0),
            .pos = rect.pos(),
            .srcRect = LRect(0, fractionalFb.sizeB()),
            .dstSize = rect.size(),
            .srcTransform = LFramebuffer::Normal,
            .srcScale = 1.f
        });

        glDisable(GL_BLEND);

        if (stateFlags.check(HasDamage))
            painter->drawRegion(damage);
        else
            painter->drawRect(LRect(0, output->currentMode()->sizeB()));

        stateFlags.add(UsingFractionalScale);
        transform = prevTrasform;
        scale = prevScale;
        rect.setPos(prevPos);
        updateRect();
    }

    stateFlags.remove(HasDamage);
    compositor()->flushClients();
    compositor()->imp()->destroyPendingRenderBuffers(&output->imp()->threadId);
    compositor()->imp()->destroyNativeTextures(nativeTexturesToDestroy);

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendResizeGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state == LOutput::ChangingMode)
    {
        output->imp()->state = LOutput::Initialized;
        output->setScale(output->fractionalScale());
        output->imp()->updateRect();
        output->imp()->updateGlobals();
        cursor()->imp()->textureChanged = true;
    }

    if (output->imp()->state != LOutput::Initialized)
        return;

    if (callLock)
        compositor()->imp()->lock();

    output->resizeGL();

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendUninitializeGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state != LOutput::PendingUninitialize)
       return;

    if (callLock)
       compositor()->imp()->lock();

    output->uninitializeGL();
    compositor()->flushClients();
    output->imp()->state = LOutput::Uninitialized;
    compositor()->imp()->destroyPendingRenderBuffers(&output->imp()->threadId);
    compositor()->imp()->destroyNativeTextures(nativeTexturesToDestroy);

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendPageFlipped()
{
    pageflipMutex.lock();
    stateFlags.add(HasUnhandledPresentationTime);
    pageflipMutex.unlock();
}

void LOutput::LOutputPrivate::updateRect()
{
    if (stateFlags.check(UsingFractionalScale))
    {
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();
        sizeB.setW(roundf(Float32(sizeB.w()) * scale / fractionalScale));
        sizeB.setH(roundf(Float32(sizeB.h()) * scale / fractionalScale));
    }
    else
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();

    // Swap width with height
    if (LFramebuffer::is90Transform(transform))
    {
        Int32 tmpW = sizeB.w();
        sizeB.setW(sizeB.h());
        sizeB.setH(tmpW);
    }

    rect.setSize(sizeB);
    rect.setW(roundf(Float32(rect.w())/scale));
    rect.setH(roundf(Float32(rect.h())/scale));
}

void LOutput::LOutputPrivate::updateGlobals()
{
    for (LClient *c : compositor()->clients())
        for (GOutput *global : c->outputGlobals())
            if (output == global->output())
                global->sendConfiguration();

    for (LSurface *s : compositor()->surfaces())
        s->imp()->sendPreferredScale();
}
