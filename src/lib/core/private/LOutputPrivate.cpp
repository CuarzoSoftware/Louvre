#include "LLog.h"
#include <iostream>
#include <protocols/Wayland/private/GOutputPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSurfacePrivate.h>

#include <LTime.h>

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

    output->setScale(output->scale());
    output->imp()->rect.setBR(output->sizeB()/output->scale());

    cursor()->imp()->textureChanged = true;
    cursor()->imp()->update();
    output->imp()->state = LOutput::Initialized;
    output->initializeGL();
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    if (output->imp()->state != LOutput::Initialized)
        return;

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }

    compositor()->imp()->renderMutex.lock();
    output->paintGL();
    destroyPendingFramebuffers();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendResizeGL()
{
    while(output->imp()->state == LOutput::ChangingMode)
    {
        output->imp()->state = LOutput::Initialized;
        output->setScale(output->scale());
    }

    if (output->imp()->state != LOutput::Initialized)
        return;

    output->resizeGL();

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }
}

void LOutput::LOutputPrivate::backendUninitializeGL()
{
    if (output->imp()->state != LOutput::Initialized)
        return;

    compositor()->imp()->renderMutex.lock();
    output->uninitializeGL();
    destroyPendingFramebuffers();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendPageFlipped()
{
    if (output->imp()->state != LOutput::Initialized)
        return;

    compositor()->imp()->renderMutex.lock();

    // Send presentation time feedback
    presentationTime = LTime::ns();
    for (LSurface *surf : compositor()->surfaces())
        surf->imp()->sendPresentationFeedback(output, presentationTime);

    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::destroyPendingFramebuffers()
{
    while (!framebuffersToDestroy.empty())
    {
        glDeleteTextures(1, &framebuffersToDestroy.back().textureId);
        glDeleteFramebuffers(1, &framebuffersToDestroy.back().framebufferId);
        framebuffersToDestroy.pop_back();
    }
}
