#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCursorPrivate.h>

#include <protocols/Wayland/GOutput.h>
#include <protocols/GammaControl/RGammaControl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <LToplevelRole.h>
#include <LRegion.h>
#include <LSeat.h>
#include <LOutputMode.h>
#include <LTime.h>
#include <LLog.h>
#include <LOutputFramebuffer.h>

using namespace Louvre;

LOutput::LOutput(const void *params) noexcept : LFactoryObject(FactoryObjectType), m_imp(std::make_unique<LOutputPrivate>(this))
{
    imp()->output = this;
    imp()->rect.setX(0);
    imp()->rect.setY(0);
    imp()->callLock.store(true);

    Params *p = (Params*)params;

    imp()->graphicBackendData = p->backendData;

    if (p->callback)
        p->callback(this);
}

LOutput::~LOutput() {}

LSessionLockRole *LOutput::sessionLockRole() const noexcept
{
    return imp()->sessionLockRole;
}

bool LOutput::needsFullRepaint() const noexcept
{
    return imp()->stateFlags.check(LOutput::LOutputPrivate::NeedsFullRepaint);
}

const LRect &LOutput::availableGeometry() const noexcept
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

bool LOutput::fractionalOversamplingEnabled() const noexcept
{
    return imp()->stateFlags.check(LOutputPrivate::FractionalOversamplingEnabled);
}

bool LOutput::usingFractionalScale() const noexcept
{
    return imp()->stateFlags.check(LOutputPrivate::UsingFractionalScale);
}

void LOutput::enableFractionalOversampling(bool enabled) noexcept
{
    if (imp()->stateFlags.check(LOutputPrivate::FractionalOversamplingEnabled) != enabled)
    {
        imp()->stateFlags.setFlag(LOutputPrivate::FractionalOversamplingEnabled, enabled);

        if (usingFractionalScale())
            repaint();
    }
}

Float32 LOutput::fractionalScale() const noexcept
{
    return imp()->fractionalScale;
}

LOutput::SubPixel LOutput::subPixel() const noexcept
{
    return (LOutput::SubPixel)compositor()->imp()->graphicBackend->outputGetSubPixel((LOutput*)this);
}

bool LOutput::hasVSyncControlSupport() const noexcept
{
    return compositor()->imp()->graphicBackend->outputHasVSyncControlSupport((LOutput*)this);
}

bool LOutput::vSyncEnabled() const noexcept
{
    return compositor()->imp()->graphicBackend->outputIsVSyncEnabled((LOutput*)this);
}

bool LOutput::enableVSync(bool enabled) noexcept
{
    return compositor()->imp()->graphicBackend->outputEnableVSync((LOutput*)this, enabled);
}

Int32 LOutput::refreshRateLimit() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetRefreshRateLimit((LOutput*)this);
}

void LOutput::setRefreshRateLimit(Int32 hz) noexcept
{
    return compositor()->imp()->graphicBackend->outputSetRefreshRateLimit((LOutput*)this, hz);
}

UInt32 LOutput::gammaSize() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetGammaSize((LOutput*)this);
}

bool LOutput::setGamma(const LGammaTable *gamma) noexcept
{
    if (gamma)
    {
        if (gamma->size() != gammaSize())
            return false;

        if (imp()->gammaTable.m_gammaControlResource && imp()->gammaTable.m_gammaControlResource != gamma->m_gammaControlResource)
            imp()->gammaTable.m_gammaControlResource->failed();

        imp()->gammaTable = *gamma;
        imp()->gammaTable.m_gammaControlResource = gamma->m_gammaControlResource;
    }
    else
    {
        if (imp()->gammaTable.m_gammaControlResource)
        {
            imp()->gammaTable.m_gammaControlResource->failed();
            imp()->gammaTable.m_gammaControlResource = nullptr;
        }

        imp()->gammaTable.setSize(gammaSize());
        imp()->gammaTable.fill(1.0, 1.0, 1.0);
    }

    return compositor()->imp()->graphicBackend->outputSetGamma((LOutput*)this, imp()->gammaTable);
}

LFramebuffer *LOutput::framebuffer() const noexcept
{
    return &imp()->fb;
}

LTransform LOutput::transform() const noexcept
{
    return imp()->transform;
}

void LOutput::setTransform(LTransform transform) noexcept
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

const std::vector<LOutputMode *> &LOutput::modes() const noexcept
{
    return *compositor()->imp()->graphicBackend->outputGetModes((LOutput*)this);
}

const LOutputMode *LOutput::preferredMode() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetPreferredMode((LOutput*)this);
}

const LOutputMode *LOutput::currentMode() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetCurrentMode((LOutput*)this);
}

void LOutput::setMode(const LOutputMode *mode) noexcept
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
    compositor()->imp()->graphicBackend->outputSetMode(this, (LOutputMode*)mode);
    imp()->state = Initialized;
    imp()->callLock.store(true);
}

Int32 LOutput::currentBuffer() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetCurrentBufferIndex((LOutput*)this);
}

UInt32 LOutput::buffersCount() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetBuffersCount((LOutput*)this);
}

LTexture *LOutput::bufferTexture(UInt32 bufferIndex) noexcept
{
    return compositor()->imp()->graphicBackend->outputGetBuffer((LOutput*)this, bufferIndex);
}

bool LOutput::hasBufferDamageSupport() const noexcept
{
    return compositor()->imp()->graphicBackend->outputHasBufferDamageSupport((LOutput*)this);
}

void LOutput::setBufferDamage(const LRegion *damage) noexcept
{
    if (!damage)
    {
        imp()->damage.clear();
        imp()->damage.addRect(rect());
        return;
    }

    imp()->damage = *damage;
}

const LRegion &LOutput::bufferDamage() const noexcept
{
    return imp()->damage;
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
    {
        imp()->stateFlags.add(LOutputPrivate::UsingFractionalScale);
        LSize fbSize = currentMode()->sizeB();
        fbSize = LSize(roundf(Float32(fbSize.w()) * imp()->scale / imp()->fractionalScale),
                             roundf(Float32(fbSize.h()) * imp()->scale / imp()->fractionalScale));
        fbSize.setW(fbSize.w() + fbSize.w() % (Int32)imp()->scale);
        fbSize.setH(fbSize.h() + fbSize.h() % (Int32)imp()->scale);
        imp()->fractionalFb.setSizeB(fbSize);
    }
    else
        imp()->stateFlags.remove(LOutputPrivate::UsingFractionalScale);

    imp()->updateRect();

    if (!imp()->stateFlags.check(LOutputPrivate::IsBlittingFramebuffers))
    {
        imp()->updateGlobals();
        cursor()->imp()->textureChanged = true;
        repaint();
        for (LSurface *s : compositor()->surfaces())
            s->imp()->sendPreferredScale();
    }
}

Float32 LOutput::scale() const noexcept
{
    return imp()->scale;
}

void LOutput::repaint() noexcept
{
    if (compositor()->imp()->graphicBackend->outputRepaint(this))
        imp()->stateFlags.add(LOutputPrivate::PendingRepaint);
}

Int32 LOutput::dpi() noexcept
{
    if (physicalSize().area() == 0)
        return 0;

    const Float64 w = imp()->sizeB.w();
    const Float64 h = imp()->sizeB.h();
    const Float64 Wi = Float64(physicalSize().w()) / 25.4f;
    const Float64 Hi = Float64(physicalSize().h()) / 25.4f;
    return sqrtf(w*w + h*h)/sqrtf(Wi*Wi + Hi*Hi);
}

const LSize &LOutput::physicalSize() const noexcept
{
    return *compositor()->imp()->graphicBackend->outputGetPhysicalSize((LOutput*)this);
}

const LSize &LOutput::sizeB() const noexcept
{
    return imp()->sizeB;
}

const LSize &LOutput::realBufferSize() const noexcept
{
    return usingFractionalScale() ? imp()->fractionalFb.sizeB() : currentMode()->sizeB();
}

const std::vector<LScreenshotRequest *> &LOutput::screenshotRequests() const noexcept
{
    return imp()->screenshotRequests;
}

LContentType LOutput::contentType() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetContentType((LOutput*)this);
}

void LOutput::setContentType(LContentType type) noexcept
{
    compositor()->imp()->graphicBackend->outputSetContentType(this, type);
}

const LRect &LOutput::rect() const noexcept
{
    return imp()->rect;
}

const LPoint &LOutput::pos() const noexcept
{
    return imp()->rect.pos();
}

const LSize &LOutput::size() const noexcept
{
    return imp()->rect.size();
}

LOutput::State LOutput::state() const noexcept
{
    return imp()->state;
}

const char *LOutput::name() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetName((LOutput*)this);
}

const char *LOutput::model() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetModelName((LOutput*)this);
}

const char *LOutput::manufacturer() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetManufacturerName((LOutput*)this);
}

const char *LOutput::description() const noexcept
{
    return compositor()->imp()->graphicBackend->outputGetDescription((LOutput*)this);
}

void LOutput::setPos(const LPoint &pos) noexcept
{
    imp()->rect.setPos(pos);
}

LPainter *LOutput::painter() const noexcept
{
    return imp()->painter;
}

const std::thread::id &LOutput::threadId() const noexcept
{
    return imp()->threadId;
}

bool LOutput::setCustomScanoutBuffer(LTexture *texture) noexcept
{
    if (!imp()->stateFlags.check(LOutputPrivate::IsInPaintGL))
    {
        LLog::error("Calling LOutput::setCustomScanoutBuffer() outside LOutput::paintGL() is not allowed.");
        return false;
    }

    if (imp()->scanout[0].buffer)
    {
        wl_list_remove(&imp()->scanout[0].bufferDestroyListener.link);
        imp()->scanout[0].buffer = nullptr;
        imp()->scanout[0].surface.reset();
        imp()->customScanoutBuffer.reset();
    }

    /* Only allow WL_DRM and DMA surface textures.
     * Scanning out surface textures from SHM buffers would require additional buffering
     * (extra CPU copies), which would actually be slower than simply rendering them.
     * User SHM buffers are allowed, though the user must take care of not updating the
     * pixels data while being scanned. */
    if (texture && texture->m_surface
        && texture->sourceType() != LTexture::DMA
        && texture->sourceType() != LTexture::WL_DRM)
    {
        imp()->stateFlags.remove(LOutputPrivate::HasScanoutBuffer);
        return false;
    }

    const bool ret { compositor()->imp()->graphicBackend->outputSetScanoutBuffer(this, texture) };

    if (ret && texture)
    {
        imp()->customScanoutBuffer.reset(texture);

        if (texture->m_surface && texture->m_surface->bufferResource())
        {
            texture->m_surface->requestNextFrame(false);
            texture->m_surface->sendOutputEnterEvent(this);
            imp()->scanout[0].buffer = texture->m_surface->bufferResource();
            imp()->scanout[0].surface.reset(texture->m_surface);
            wl_resource_add_destroy_listener(
                (wl_resource*)imp()->scanout[0].buffer,
                &imp()->scanout[0].bufferDestroyListener);
        }
    }

    imp()->stateFlags.setFlag(LOutputPrivate::HasScanoutBuffer, ret);
    return ret;
}
