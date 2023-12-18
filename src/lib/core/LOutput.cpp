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

#include <LToplevelRole.h>
#include <LRegion.h>
#include <LSeat.h>
#include <LOutputMode.h>
#include <LTime.h>
#include <LLog.h>
#include <LOutputFramebuffer.h>
#include <string>

using namespace Louvre;

LOutput::LOutput() : m_imp(std::make_unique<LOutputPrivate>(this))
{
    imp()->output = this;
    imp()->rect.setX(0);
    imp()->rect.setY(0);
    imp()->callLock.store(true);
}

LOutput::~LOutput() {}

LFramebuffer *LOutput::framebuffer() const
{
    return &imp()->fb;
}

LFramebuffer::Transform LOutput::transform() const
{
    return imp()->transform;
}

void LOutput::setTransform(LFramebuffer::Transform transform) const
{
    if (transform == imp()->transform)
        return;

    LSize prevSizeB = imp()->sizeB;
    imp()->transform = transform;
    imp()->updateRect();

    if (state() == Initialized && prevSizeB != imp()->sizeB)
        imp()->updateGlobals();
}

const std::list<LOutputMode *> &LOutput::modes() const
{
    return *compositor()->imp()->graphicBackend->getOutputModes((LOutput*)this);
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

    // Setting output mode from a rendering thread is not allowed
    for (LOutput *o : compositor()->outputs())
        if (o->threadId() == std::this_thread::get_id())
            return;

    imp()->callLockACK.store(false);
    imp()->callLock.store(false);
    compositor()->imp()->unlock();

    Int32 waitLimit = 0;

    while (!imp()->callLockACK.load() && waitLimit < 1000)
    {
        usleep(1000);
        waitLimit++;
    }

    compositor()->imp()->lock();
    imp()->state = ChangingMode;
    compositor()->imp()->graphicBackend->setOutputMode(this, (LOutputMode*)mode);
    imp()->state = Initialized;
    imp()->callLock.store(true);
}

Int32 LOutput::currentBuffer() const
{
    return compositor()->imp()->graphicBackend->getOutputCurrentBufferIndex((LOutput*)this);
}

UInt32 LOutput::buffersCount() const
{
    return compositor()->imp()->graphicBackend->getOutputBuffersCount((LOutput*)this);
}

LTexture *LOutput::bufferTexture(UInt32 bufferIndex)
{
    return compositor()->imp()->graphicBackend->getOutputBuffer((LOutput*)this, bufferIndex);
}

bool LOutput::hasBufferDamageSupport() const
{
    return compositor()->imp()->graphicBackend->hasBufferDamageSupport((LOutput*)this);
}

void LOutput::setBufferDamage(const LRegion &damage)
{
    if (!hasBufferDamageSupport())
        return;

    LRegion region(LRect(0,0,10,10));
    Int32 n, x, y, w, h;
    LBox *boxes;

    switch (transform())
    {
    case LFramebuffer::Normal:
        region = damage;
        region.offset(LPoint(-pos().x(), -pos().y()));
        region.multiply(scale());
        region.clip(0, sizeB());
        break;
    case LFramebuffer::Clock90:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->y2 - boxes->y1;
            h = boxes->x2 - boxes->x1;
            x = size().h() - (boxes->y1 - pos().y()) - w;
            y = boxes->x1 - pos().x();

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().h(), sizeB().w());
        break;
    case LFramebuffer::Clock180:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->x2 - boxes->x1;
            h = boxes->y2 - boxes->y1;
            x = size().w() - (boxes->x1 - pos().x()) - w;
            y = size().h() - (boxes->y1 - pos().y()) - h;

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().w(), sizeB().h());
        break;
    case LFramebuffer::Clock270:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->y2 - boxes->y1;
            h = boxes->x2 - boxes->x1;
            y = size().w() - (boxes->x1 - pos().x()) - h;
            x = boxes->y1 - pos().y();

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().h(), sizeB().w());
        break;
    case LFramebuffer::Flipped:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            x = boxes->x1 - pos().x();
            y = boxes->y1 - pos().y();
            w = boxes->x2 - boxes->x1;
            h = boxes->y2 - boxes->y1;
            x = size().w() - x - w;

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, sizeB());
        break;
    case LFramebuffer::Flipped90:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->y2 - boxes->y1;
            h = boxes->x2 - boxes->x1;
            x = size().h() - (boxes->y1 - pos().y()) - w;
            y = size().w() - (boxes->x1 - pos().x()) - h;

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().h(), sizeB().w());
        break;
    case LFramebuffer::Flipped180:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->x2 - boxes->x1;
            h = boxes->y2 - boxes->y1;
            x = boxes->x1 - pos().x();
            y = size().h() - (boxes->y1 - pos().y()) - h;

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().w(), sizeB().h());
        break;
    case LFramebuffer::Flipped270:
        boxes = damage.boxes(&n);

        for (Int32 i = 0; i < n; i++)
        {
            w = boxes->y2 - boxes->y1;
            h = boxes->x2 - boxes->x1;
            y = boxes->x1 - pos().x();
            x = boxes->y1 - pos().y();

            region.addRect(
                x * scale(),
                y * scale(),
                w * scale(),
                h * scale());

            boxes++;
        }

        region.clip(0, 0, sizeB().h(), sizeB().w());
        break;
    default:
        break;
    }

    compositor()->imp()->graphicBackend->setOutputBufferDamage((LOutput*)this, region);
}

void LOutput::setScale(Int32 scale)
{
    if (scale < 1 || scale > 3)
        return;

    imp()->outputScale = scale;
    imp()->updateRect();

    if (scale == imp()->outputScale)
        return;

    imp()->updateGlobals();
    compositor()->imp()->updateGreatestOutputScale();
}

Int32 LOutput::scale() const
{
    return imp()->outputScale;
}

void LOutput::repaint()
{
    if (compositor()->imp()->graphicBackend->scheduleOutputRepaint(this))
        imp()->pendingRepaint = true;
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
    return *compositor()->imp()->graphicBackend->getOutputPhysicalSize((LOutput*)this);
}

const LSize &LOutput::sizeB() const
{
    return imp()->sizeB;
}

const LRect &LOutput::rect() const
{
    return imp()->rect;
}

const LPoint &LOutput::pos() const
{
    return rect().pos();
}

const LSize &LOutput::size() const
{
    return rect().size();
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

void LOutput::setPos(const LPoint &pos)
{
    imp()->rect.setPos(pos);
}

LPainter *LOutput::painter() const
{
    return imp()->painter;
}

const std::thread::id &LOutput::threadId() const
{
    return imp()->threadId;
}
