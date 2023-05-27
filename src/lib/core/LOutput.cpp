#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCursorPrivate.h>

#include <protocols/Wayland/private/GOutputPrivate.h>

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
#include <LToplevelRole.h>
#include <LRegion.h>
#include <LSeat.h>
#include <LOutputMode.h>
#include <LTime.h>
#include <LLog.h>

using namespace Louvre;

LOutput::LOutput()
{
    m_imp = new LOutputPrivate();
    imp()->output = this;
}

LOutput::~LOutput()
{
    delete m_imp;
}

LCompositor *LOutput::compositor() const
{
    return imp()->compositor;
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
    if (mode == currentMode())
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
    imp()->outputScale = scale;
    imp()->rectC.setBR((sizeB()*compositor()->globalScale())/scale);

    for (LClient *c : compositor()->clients())
    {
        for (Protocols::Wayland::GOutput *gOutput : c->outputGlobals())
        {
            if (this == gOutput->output())
            {
                gOutput->sendConfiguration();
                break;
            }
        }
    }

    compositor()->imp()->updateGlobalScale();
}

Int32 LOutput::scale() const
{
    return imp()->outputScale;
}

// This is called from LCompositor::addOutput()
bool LOutput::LOutputPrivate::initialize(LCompositor *comp)
{
    compositor = comp;

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
    painter = new LPainter();
    painter->imp()->output = output;

    output->imp()->global = wl_global_create(LWayland::getDisplay(),
                                             &wl_output_interface,
                                             LOUVRE_OUTPUT_VERSION,
                                             output,
                                             &Protocols::Wayland::GOutput::GOutputPrivate::bind);

    output->setScale(output->scale());
    output->imp()->rectC.setBR((output->sizeB()*compositor->globalScale())/output->scale());

    compositor->cursor()->imp()->textureChanged = true;
    compositor->cursor()->imp()->update();

    compositor->imp()->updateGlobalScale();
}

void LOutput::LOutputPrivate::backendBeforePaint()
{
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
    for (LSurface *surf : compositor->surfaces())
        surf->imp()->sendPresentationFeedback(output, presentationTime);
}

void LOutput::repaint()
{
    compositor()->imp()->graphicBackend->scheduleOutputRepaint(this);
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
    return *imp()->compositor->imp()->graphicBackend->getOutputPhysicalSize((LOutput*)this);
}

const LSize &LOutput::sizeB() const
{
    return currentMode()->sizeB();
}

const LRect &LOutput::rectC() const
{
    return imp()->rectC;
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
    return imp()->state;
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
    imp()->rectC.setX(posC.x());
    imp()->rectC.setY(posC.y());
}

LPainter *LOutput::painter() const
{
    return imp()->painter;
}
