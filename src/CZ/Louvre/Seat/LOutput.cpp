#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LLockGuard.h>

#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Protocols/GammaControl/RZwlrGammaControlV1.h>
#include <CZ/Louvre/Protocols/DRMLease/GDRMLeaseDevice.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseConnector.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLease.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputMode.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Cursor/LCursor.h>

#include <CZ/skia/core/SkRegion.h>

using namespace CZ;

LOutput::LOutput(const void *params) noexcept : LFactoryObject(FactoryObjectType), m_imp(std::make_unique<LOutputPrivate>(this))
{
    Params *p = (Params*)params;
    m_backend = p->backend;
    m_backend->m_output = this;
    imp()->updateRect();
}

LOutput::~LOutput() noexcept
{
    notifyDestruction();
}

LSessionLockRole *LOutput::sessionLockRole() const noexcept
{
    return imp()->sessionLockRole;
}

bool LOutput::needsFullRepaint() const noexcept
{
    return imp()->stateFlags.has(LOutput::LOutputPrivate::NeedsFullRepaint);
}

const SkIRect &LOutput::availableGeometry() const noexcept
{
    return imp()->availableGeometry;
}

const LMargins &LOutput::exclusiveEdges() const noexcept
{
    return imp()->exclusiveEdges;
}

const std::list<LExclusiveZone *> LOutput::exclusiveZones() const noexcept
{
    return imp()->exclusiveZones;
}

bool LOutput::usingFractionalScale() const noexcept
{
    return imp()->stateFlags.has(LOutputPrivate::UsingFractionalScale);
}

bool LOutput::oversamplingEnabled() const noexcept
{
    return imp()->stateFlags.has(LOutputPrivate::OversamplingEnabled);
}

void LOutput::enableOversampling(bool enabled) noexcept
{
    if (imp()->stateFlags.has(LOutputPrivate::OversamplingEnabled) != enabled)
    {
        imp()->stateFlags.setFlag(LOutputPrivate::OversamplingEnabled, enabled);

        if (usingFractionalScale())
            repaint();
    }
}

Float32 LOutput::fractionalScale() const noexcept
{
    return imp()->fractionalScale;
}

bool LOutput::setGammaLUT(std::shared_ptr<const RGammaLUT> gamma) noexcept
{
    const bool ok { backend()->setGammaLUT(gamma) };

    if (ok && imp()->wlrGammaControl)
    {
        imp()->wlrGammaControl->failed();
        imp()->wlrGammaControl.reset();
    }

    return ok;
}

std::shared_ptr<RImage> LOutput::backendImage() const noexcept
{
    return images()[imageIndex()];
}

std::shared_ptr<RImage> LOutput::osImage() const noexcept
{
    if (imp()->osSurface)
        return imp()->osSurface->image();

    return {};
}

CZTransform LOutput::transform() const noexcept
{
    return imp()->transform;
}

void LOutput::setTransform(CZTransform transform) noexcept
{
    if (transform == imp()->transform)
        return;

    const auto prevBufferSize { imp()->bufferSize };
    imp()->transform = transform;
    imp()->updateRect();

    if (state() == Initialized && prevBufferSize != imp()->bufferSize)
    {
        repaint();
        imp()->updateGlobals();
        cursor()->m_imageChanged = true;
    }

    for (auto *head : imp()->wlrOutputHeads)
        head->transform(transform);
}

UInt32 LOutput::imageAge() const noexcept
{
    if (needsFullRepaint())
        return 0;

    return m_backend->imageAge();
}

std::shared_ptr<RImage> LOutput::image() const noexcept
{
    if (oversamplingEnabled() && usingFractionalScale())
        return osImage();

    return backendImage();
}

int LOutput::setMode(std::shared_ptr<LOutputMode> mode) noexcept
{
    if (!mode)
        return 0;

    if (mode->output() != this)
        return 0;

    if (mode == currentMode())
        return 1;

    if (imp()->state == Uninitialized)
        return m_backend->setMode(mode);

    // Setting output mode from a rendering thread is not allowed
    for (LOutput *o : compositor()->outputs())
        if (o->threadId() == std::this_thread::get_id())
            return 0;

    imp()->state = ChangingMode;
    const auto unlocked { LLockGuard::Unlock() };
    const auto ret { m_backend->setMode(mode) };
    imp()->state = Initialized;

    if (unlocked)
        LLockGuard::Lock();

    if (ret != 1)
        return ret;

    for (auto *head : imp()->wlrOutputHeads)
    {
        for (auto *mode : head->modes())
        {
            if (mode->mode() == currentMode())
            {
                head->currentMode(mode);
                break;
            }
        }
    }

    return ret;
}

void LOutput::setScale(Float32 scale) noexcept
{
    if (scale < 0.25f)
        scale = 0.25f;

    if (imp()->fractionalScale == scale)
        return;

    imp()->scale = ceilf(scale);
    imp()->fractionalScale = scale;

    if (fmod(imp()->fractionalScale, 1.f) != 0.f)
        imp()->stateFlags.add(LOutputPrivate::UsingFractionalScale);
    else
        imp()->stateFlags.remove(LOutputPrivate::UsingFractionalScale);

    imp()->updateRect();

    if (!imp()->stateFlags.has(LOutputPrivate::IsBlittingFramebuffers))
    {
        imp()->updateGlobals();
        cursor()->m_imageChanged = true;
        repaint();
        for (LSurface *s : compositor()->surfaces())
            s->imp()->sendPreferredScale();
    }

    for (auto *head : imp()->wlrOutputHeads)
        head->scale(imp()->fractionalScale);
}

Float32 LOutput::scale() const noexcept
{
    return imp()->scale;
}

void LOutput::repaint() noexcept
{
    if (m_backend->repaint())
        imp()->stateFlags.add(LOutputPrivate::PendingRepaint);
}

Int32 LOutput::dpi() noexcept
{
    // Virtual drivers don't provide a physical size, so fallback to a common low DPI value
    if (mmSize().area() == 0)
        return 96;

    const Float64 pixelWidth = imp()->bufferSize.width();
    const Float64 pixelHeight = imp()->bufferSize.height();
    const Float64 inchWidth = Float64(mmSize().width()) / 25.4f;
    const Float64 inchHeight = Float64(mmSize().height()) / 25.4f;

    return sqrtf(pixelWidth * pixelWidth + pixelHeight * pixelHeight) /
           sqrtf(inchWidth * inchWidth + inchHeight * inchHeight);
}

SkISize LOutput::sizeB() const noexcept
{
    return imp()->bufferSize;
}

SkISize LOutput::realBufferSize() const noexcept
{
    //TODO:return usingFractionalScale() ? imp()->fractionalFb.sizeB() : currentMode()->sizeB();
    return currentMode()->size();
}

void LOutput::setLeasable(bool leasable) noexcept
{
    if (leasable == imp()->leasable)
        return;

    imp()->leasable = leasable;

    if (leasable)
    {
        for (LClient *client : compositor()->clients())
        {
            for (auto *global : client->drmLeaseDeviceGlobals())
            {
                if (global->device() == device())
                {
                    if (global->connector(this))
                        global->done();
                    break;
                }
            }
        }
    }
    else
    {
        if (lease())
            lease()->finished();

        for (auto *drmLeaseConnRes : imp()->drmLeaseConnectorRes)
        {
            drmLeaseConnRes->withdrawn();
            drmLeaseConnRes->done();
        }
    }
}

bool LOutput::leasable() noexcept
{
    return imp()->leasable;
}

DRMLease::RDRMLease *LOutput::lease() const noexcept
{
    return imp()->lease;
}

const SkIRect &LOutput::rect() const noexcept
{
    return imp()->rect;
}

SkIPoint LOutput::pos() const noexcept
{
    return imp()->rect.topLeft();
}

SkISize LOutput::size() const noexcept
{
    return imp()->rect.size();
}

LOutput::State LOutput::state() const noexcept
{
    return imp()->state;
}

void LOutput::setPos(SkIPoint pos) noexcept
{
    imp()->rect.offsetTo(pos.x(), pos.y());

    for (auto *head : imp()->wlrOutputHeads)
        head->position(pos);
}

const std::thread::id &LOutput::threadId() const noexcept
{
    return imp()->threadId;
}
