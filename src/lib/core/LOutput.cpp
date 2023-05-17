#include "LLog.h"
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LSurfacePrivate.h>

#include <protocols/Wayland/Output.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <LWayland.h>
#include <LCursor.h>
#include <LToplevelRole.h>
#include <LRegion.h>
#include <LSeat.h>
#include <LOutputMode.h>
#include <LTime.h>

using namespace Louvre;

LOutput::LOutput()
{
    m_imp = new LOutputPrivate();
    m_imp->output = this;
}

LOutput::~LOutput()
{
    delete m_imp;
}

LCompositor *LOutput::compositor() const
{
    return m_imp->compositor;
}

LCursor *LOutput::cursor() const
{
    return compositor()->cursor();
}

LSeat *LOutput::seat() const
{
    return compositor()->seat();
}

const list<LOutputMode *> *LOutput::modes() const
{
    return compositor()->imp()->graphicBackend->getOutputModes((LOutput*)this);
}

const LOutputMode *LOutput::preferredMode() const
{
    return compositor()->imp()->graphicBackend->getOutputPreferredMode((LOutput*)this);
}

const LOutputMode *LOutput::currentMode() const
{
    return compositor()->imp()->graphicBackend->getOutputCurrentMode((LOutput*)this);
}

void LOutput::setMode(const LOutputMode *mode)
{
    if(mode == currentMode())
        return;

    imp()->rectC.setW((mode->sizeB().w()*compositor()->globalScale())/scale());
    imp()->rectC.setH((mode->sizeB().h()*compositor()->globalScale())/scale());

    setScale(scale());

    imp()->pendingMode = (LOutputMode*)mode;
    imp()->state = ChangingMode;

    while(imp()->state != Initialized)
    {
        repaint();
        usleep(100);
    }
}

Int32 LOutput::currentBuffer() const
{
    return compositor()->imp()->graphicBackend->getOutputCurrentBufferIndex((LOutput*)this);
}

void LOutput::setScale(Int32 scale)
{
    m_imp->outputScale = scale;

    m_imp->rectC.setBR((sizeB()*compositor()->globalScale())/scale);

    // Notifica a los globals wl_output de cada cliente
    for(LClient *c : compositor()->clients())
    {
        for(wl_resource *r : c->outputs())
        {
            LOutput *o = (LOutput*)wl_resource_get_user_data(r);

            if(this == o)
            {
                Globals::Output::sendConfiguration(r,o);
                break;
            }
        }
    }

    compositor()->imp()->updateGlobalScale();
}

Int32 LOutput::scale() const
{
    return m_imp->outputScale;
}

// This is called from LCompositor::addOutput()
bool LOutput::LOutputPrivate::initialize(LCompositor *comp)
{
    compositor = comp;
    output->imp()->rectC.setBR((output->sizeB()*comp->globalScale())/output->scale());
    output->setScale(output->scale());

    // The backend must call LOutputPrivate::backendInitialized() before initializeGL()
    return compositor->imp()->graphicBackend->initializeOutput(output);
}

void LOutput::LOutputPrivate::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    rectC.setSize((output->currentMode()->sizeB()*newScale)/output->scale());
}

void LOutput::LOutputPrivate::backendInitialized()
{
    //LWayland::bindEGLDisplay(compositor->imp()->graphicBackend->getOutputEGLDisplay(output));

    painter = new LPainter();
    painter->imp()->output = output;

    // Create cursor
    if(!compositor->cursor())
    {
        compositor->imp()->cursor = new LCursor(output);
        compositor->cursorInitialized();
    }

    output->imp()->global = wl_global_create(LWayland::getDisplay(), &wl_output_interface, LOUVRE_OUTPUT_VERSION, output, &Louvre::Globals::Output::bind);

}

void LOutput::LOutputPrivate::backendBeforePaint()
{
    scheduledRepaint = false;
    output->imp()->compositor->imp()->renderMutex.lock();
}

void LOutput::LOutputPrivate::backendAfterPaint()
{
    output->imp()->compositor->imp()->renderMutex.unlock();
    LWayland::flushClients();
}

void LOutput::LOutputPrivate::backendPageFlipped()
{
    output->imp()->presentationSeq++;

    // Send presentation time feedback
    presentationTime = LTime::ns();
    for(LSurface *surf : output->compositor()->surfaces())
        surf->imp()->sendPresentationFeedback(output, presentationTime);
}
/*
void LOutput::LOutputPrivate::renderLoop(void *data)
{
    LOutput *output = (LOutput*)data;
    output->imp()->scheduledRepaint = false;
    output->repaint();
    timespec presentationTime;

    while(true)
    {
        // Wait for a repaint request
        poll(&output->imp()->renderPoll, 1, -1);

        if(!output->compositor()->seat()->enabled())
        {
            output->imp()->scheduledRepaint = false;
            eventfd_read(output->imp()->renderPoll.fd, &output->imp()->renderValue);
            continue;
        }
        else if(output->imp()->state == LOutput::PendingUninitialize)
        {
            output->uninitializeGL();
            output->imp()->state = LOutput::Uninitialized;
            delete output->imp()->painter;
            return;
        }
        else if(output->imp()->state == LOutput::ChangingMode)
        {
            output->compositor()->imp()->graphicBackend->setOutputMode(output, output->imp()->pendingMode);
            output->imp()->state = LOutput::Initialized;
            output->resizeGL();
            continue;
        }

        output->imp()->scheduledRepaint = false;

        eventfd_read(output->imp()->renderPoll.fd, &output->imp()->renderValue);

        output->paintGL();



        output->imp()->compositor->imp()->graphicBackend->flipOutputPage(output);


    }
}
*/

void LOutput::repaint()
{
    if(!imp()->scheduledRepaint)
    {
        imp()->scheduledRepaint = true;
        compositor()->imp()->graphicBackend->scheduleOutputRepaint(this);
    }
}

Int32 LOutput::dpi()
{
    float w = sizeB().w();
    float h = sizeB().h();

    float Wi = physicalSize().w();
    Wi /= 25.4;
    float Hi = physicalSize().h();
    Hi /= 25.4;

    return sqrtf(w*w+h*h)/sqrtf(Wi*Wi+Hi*Hi);
}

const LSize &LOutput::physicalSize() const
{
    return *m_imp->compositor->imp()->graphicBackend->getOutputPhysicalSize((LOutput*)this);
}

const LSize &LOutput::sizeB() const
{
    return currentMode()->sizeB();
}

const LRect &LOutput::rectC() const
{
    return m_imp->rectC;
}

const LPoint &LOutput::posC() const
{
    return rectC().topLeft();
}

const LSize &LOutput::sizeC() const
{
    return rectC().bottomRight();
}

EGLDisplay LOutput::eglDisplay()
{
    return compositor()->imp()->graphicBackend->getOutputEGLDisplay(this);
}

LOutput::State LOutput::state() const
{
    return m_imp->state;
}

const char *LOutput::name() const
{
    return compositor()->imp()->graphicBackend->getOutputName((LOutput*)this);
}

const char *LOutput::model() const
{
    return compositor()->imp()->graphicBackend->getOutputModelName((LOutput*)this);
}

const char *LOutput::manufacturer() const
{
    return compositor()->imp()->graphicBackend->getOutputManufacturerName((LOutput*)this);
}

const char *LOutput::description() const
{
    return compositor()->imp()->graphicBackend->getOutputDescription((LOutput*)this);
}

void LOutput::setPosC(const LPoint &posC)
{
    m_imp->rectC.setX(posC.x());
    m_imp->rectC.setY(posC.y());
}

LPainter *LOutput::painter() const
{
    return m_imp->painter;
}

LOutput::LOutputPrivate *LOutput::imp() const
{
    return m_imp;
}


