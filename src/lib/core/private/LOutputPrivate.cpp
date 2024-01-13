#include <protocols/Wayland/private/GOutputPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSurfacePrivate.h>

#include <LTime.h>
#include <iostream>

LOutput::LOutputPrivate::LOutputPrivate(LOutput *output) : fb(output), fractionalFb(LSize(64, 64), false) {}

// This is called from LCompositor::addOutput()
bool LOutput::LOutputPrivate::initialize()
{
    output->imp()->state = LOutput::PendingInitialize;
    // The backend must call LOutputPrivate::backendInitialized() before initializeGL()
    return compositor()->imp()->graphicBackend->initializeOutput(output);
}

void LOutput::LOutputPrivate::backendInitializeGL()
{
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

    output->paintGL();

    if (stateFlags.check(HasDamage) && (stateFlags.checkAll(UsingFractionalScale | FractionalOversamplingEnabled) || output->hasBufferDamageSupport()))
    {
        damage.offset(-rect.pos().x(), -rect.pos().y());
        damage.transform(rect.size(), transform);
        damage.multiply(scale/(scale/fractionalScale));
        damage.clip(LRect(0, output->currentMode()->sizeB()));

        if (output->hasBufferDamageSupport())
            compositor()->imp()->graphicBackend->setOutputBufferDamage(output, damage);
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
        output->setScale(output->scale());
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
    presentationTime = LTime::ns();
    stateFlags.add(HasUnhandledPresentationTime);
    pageflipMutex.unlock();
}

void LOutput::LOutputPrivate::updateRect()
{
    if (stateFlags.check(UsingFractionalScale))
    {
        sizeB = compositor()->imp()->graphicBackend->getOutputCurrentMode(output)->sizeB();
        sizeB.setW(roundf(Float32(sizeB.w()) * scale / fractionalScale));
        sizeB.setH(roundf(Float32(sizeB.h()) * scale / fractionalScale));
    }
    else
        sizeB = compositor()->imp()->graphicBackend->getOutputCurrentMode(output)->sizeB();

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
    {
        for (GOutput *gOutput : c->outputGlobals())
        {
            if (output == gOutput->output())
            {
                gOutput->sendConfiguration();
                break;
            }
        }
    }
}
