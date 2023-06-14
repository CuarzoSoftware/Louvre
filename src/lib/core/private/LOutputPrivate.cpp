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
    // The backend must call LOutputPrivate::backendInitialized() before initializeGL()
    return compositor()->imp()->graphicBackend->initializeOutput(output);
}

void LOutput::LOutputPrivate::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);
    rectC.setSize((output->sizeB()*newScale)/output->scale());
}

void LOutput::LOutputPrivate::backendInitializeGL()
{
    painter = new LPainter();
    painter->imp()->output = output;

    output->imp()->global = wl_global_create(compositor()->display(),
                                             &wl_output_interface,
                                             LOUVRE_WL_OUTPUT_VERSION,
                                             output,
                                             &Protocols::Wayland::GOutput::GOutputPrivate::bind);

    output->setScale(output->scale());
    output->imp()->rectC.setBR((output->sizeB()*compositor()->globalScale())/output->scale());

    cursor()->imp()->textureChanged = true;
    cursor()->imp()->update();
    compositor()->imp()->updateGlobalScale();

    compositor()->imp()->renderMutex.lock();
    output->initializeGL();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    compositor()->imp()->renderMutex.lock();
    output->paintGL();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendResizeGL()
{
    compositor()->imp()->renderMutex.lock();
    output->resizeGL();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendUninitializeGL()
{
    compositor()->imp()->renderMutex.lock();
    output->uninitializeGL();
    compositor()->imp()->renderMutex.unlock();
}

void LOutput::LOutputPrivate::backendPageFlipped()
{
    bool running = output->state() == LOutput::Initialized;

    if (running)
        compositor()->imp()->renderMutex.lock();

    // Send presentation time feedback
    output->imp()->presentationSeq++;
    presentationTime = LTime::ns();
    for (LSurface *surf : compositor()->surfaces())
        surf->imp()->sendPresentationFeedback(output, presentationTime);

    if (running)
        compositor()->imp()->renderMutex.unlock();
}
