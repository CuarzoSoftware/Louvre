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

bool LOutput::fractionalOversamplingEnabled() const
{
    return imp()->stateFlags.check(LOutputPrivate::FractionalOversamplingEnabled);
}

bool LOutput::usingFractionalScale() const
{
    return imp()->stateFlags.check(LOutputPrivate::UsingFractionalScale);
}

void LOutput::enableFractionalOversampling(bool enabled)
{
    imp()->stateFlags.setFlag(LOutputPrivate::FractionalOversamplingEnabled, enabled);
}

Float32 LOutput::fractionalScale() const
{
    return imp()->fractionalScale;
}

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

void LOutput::setTransform(LFramebuffer::Transform transform)
{
    if (transform == imp()->transform)
        return;

    LSize prevSizeB = imp()->sizeB;
    imp()->transform = transform;
    imp()->updateRect();

    if (state() == Initialized && prevSizeB != imp()->sizeB)
    {
        repaint();
        imp()->updateGlobals();
        cursor()->imp()->textureChanged = true;
    }
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

void LOutput::setBufferDamage(const LRegion *damage)
{
    if (!damage)
    {
        imp()->stateFlags.remove(LOutputPrivate::HasDamage);
        return;
    }

    imp()->damage = *damage;
    imp()->stateFlags.add(LOutputPrivate::HasDamage);

    /*
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
    case LFramebuffer::Rotated270:
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
    case LFramebuffer::Rotated180:
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
    case LFramebuffer::Rotated90:
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
    case LFramebuffer::Flipped270:
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
    case LFramebuffer::Flipped90:
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
*/
}

void LOutput::setScale(Float32 scale)
{
    if (scale < 0.25f)
        scale = 0.25f;

    if (imp()->fractionalScale == scale)
        return;

    imp()->scale = ceilf(scale);
    imp()->fractionalScale = scale;

    if (fmod(imp()->fractionalScale, 1.f) != 0.f)
    {
        imp()->stateFlags.add(LOutputPrivate::UsingFractionalScale);
        LSize fbSize = currentMode()->sizeB();
        fbSize = LSize(roundf(Float32(fbSize.w()) * imp()->scale / imp()->fractionalScale),
                             roundf(Float32(fbSize.h()) * imp()->scale / imp()->fractionalScale));
        imp()->fractionalFb.setSizeB(fbSize);
    }
    else
        imp()->stateFlags.remove(LOutputPrivate::UsingFractionalScale);

    imp()->updateRect();
    imp()->updateGlobals();
    cursor()->imp()->textureChanged = true;

    for (LSurface *s : compositor()->surfaces())
        s->imp()->sendPreferredScale();
}

Float32 LOutput::scale() const
{
    return imp()->scale;
}

void LOutput::repaint()
{
    if (compositor()->imp()->graphicBackend->scheduleOutputRepaint(this))
        imp()->stateFlags.add(LOutputPrivate::PendingRepaint);
}

Int32 LOutput::dpi()
{
    float w = imp()->sizeB.w();
    float h = imp()->sizeB.h();

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
    return imp()->rect.pos();
}

const LSize &LOutput::size() const
{
    return imp()->rect.size();
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
