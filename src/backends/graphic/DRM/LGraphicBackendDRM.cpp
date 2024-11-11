#include <LLog.h>
#include <sys/epoll.h>

#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>

#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm.h>
#include <drm_fourcc.h>

#include <LGraphicBackend.h>
#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LFactory.h>

#include <LGPU.h>
#include <LTime.h>
#include <LGammaTable.h>
#include <LOutputMode.h>

#include <private/SRMBufferPrivate.h>
#include <private/SRMConnectorPrivate.h>
#include <private/SRMCrtcPrivate.h>
#include <private/SRMPlanePrivate.h>
#include <private/SRMEncoderPrivate.h>
#include <private/SRMDevicePrivate.h>
#include <SRMCore.h>
#include <SRMConnectorMode.h>
#include <SRMListener.h>
#include <SRMList.h>
#include <SRMFormat.h>

using namespace Louvre;

#define SRM_VERSION_GREATER_EQUAL_THAN(major, minor, patch) (major < SRM_VERSION_MAJOR || (major == SRM_VERSION_MAJOR && (minor < SRM_VERSION_MINOR || (minor == SRM_VERSION_MINOR && patch <= SRM_VERSION_PATCH))))
#define SRM_VERSION_LESS_THAN(major, minor, patch) !SRM_VERSION_GREATER_EQUAL_THAN(major, minor, patch)

#define BKND_NAME "DRM BACKEND"

static bool libseatEnabled { false };

class DRMLease final : public LObject
{
public:
    int fd;
    UInt32 lessee;
    LGPU *gpu { nullptr };
    std::vector<SRMConnector*> connectors;
    std::vector<SRMCrtc*> crtcs;
    std::vector<SRMPlane*> planes;
    std::vector<SRMEncoder*> encoders;

    ~DRMLease()
    {
        if (fd >= 0)
        {
            // This could fail if not master (TTY switch)
            // Proper clean up is done in backendResume()
            int ret = drmModeRevokeLease(gpu->fd(), lessee);

            if (ret != 0)
                LLog::error("[%s] drmModeRevokeLease failed (%d). Cleaning up resources once the session (DRM master) is restored.", BKND_NAME, ret);
        }

        while (!connectors.empty())
        {
            connectors.back()->currentCrtc = NULL;
            connectors.back()->currentPrimaryPlane = NULL;
            connectors.back()->currentCursorPlane = NULL;
            connectors.back()->currentEncoder = NULL;
            connectors.pop_back();
        }

        while (!crtcs.empty())
        {
            crtcs.back()->currentConnector = NULL;
            crtcs.pop_back();
        }

        while (!planes.empty())
        {
            planes.back()->currentConnector = NULL;
            planes.pop_back();
        }

        while (!encoders.empty())
        {
            encoders.back()->currentConnector = NULL;
            encoders.pop_back();
        }
    }
};

struct Backend
{
    SRMCore *core;
    std::vector<LOutput*>connectedOutputs;
    wl_event_source *monitor;
    std::vector<LDMAFormat>dmaFormats;
    std::vector<LDMAFormat>scanoutFormats;
    std::vector<LGPU*> devices;
    LWeak<LGPU> allocator;
};

struct Output
{
    SRMConnector *conn;
    LSize physicalSize;
    std::vector<LOutputMode*>modes;
    std::vector<LTexture*> textures;
    LWeak<DRMLease> lease;
    std::string description;
};

// SRM -> Louvre Subpixel table
static UInt32 subPixelTable[] =
{
    0,
    LOutput::SubPixel::Unknown,
    LOutput::SubPixel::HorizontalRGB,
    LOutput::SubPixel::HorizontalBGR,
    LOutput::SubPixel::VerticalRGB,
    LOutput::SubPixel::VerticalBGR,
    LOutput::SubPixel::None,
};

int LGraphicBackend::openRestricted(const char *path, int flags, void *userData)
{
    LCompositor *compositor {(LCompositor*)userData};
    Backend *bknd {(Backend*)compositor->imp()->graphicBackendData};

    LGPU *dev = new LGPU;
    struct stat stat;
    dev->m_name = path;

    if (libseatEnabled)
    {
        dev->m_id = compositor->seat()->openDevice(path, &dev->m_fd);

        if (dev->m_id == -1)
        {
            delete dev;
            return -1;
        }
        else
        {
            dev->m_roFd = open(path, O_RDONLY | O_CLOEXEC);
            bknd->devices.push_back(dev);

            if (fstat(dev->fd(), &stat) == 0)
                dev->m_dev = stat.st_rdev;
            else
            {
                dev->m_dev = -1;
                LLog::fatal("[%s] Failed to get allocator device ID.", BKND_NAME);
            }

            return dev->m_fd;
        }
    }
    else
    {
        dev->m_fd = open(path, flags);
        dev->m_roFd = open(path, O_RDONLY | O_CLOEXEC);
        bknd->devices.push_back(dev);

        if (fstat(dev->fd(), &stat) == 0)
            dev->m_dev = stat.st_rdev;
        else
        {
            dev->m_dev = -1;
            LLog::fatal("[%s] Failed to get allocator device ID.", BKND_NAME);
        }

        return dev->m_fd;
    }
}

void LGraphicBackend::closeRestricted(int fd, void *userData)
{
    LCompositor *compositor {(LCompositor*)userData};
    Backend *bknd {(Backend*)compositor->imp()->graphicBackendData};

    std::unique_ptr<LGPU> dev { nullptr };

    for (std::size_t i = 0; i < bknd->devices.size(); i++)
    {
        if (bknd->devices[i]->fd() == fd)
        {
            dev.reset(bknd->devices[i]);
            bknd->devices[i] = bknd->devices.back();
            bknd->devices.pop_back();
            break;
        }
    }

    if (!dev)
        goto closeFD;

    close(dev->roFd());

    if (libseatEnabled)
        compositor->seat()->closeDevice(dev->id());

closeFD:
    close(fd);
}

static SRMInterface srmInterface =
{
    .openRestricted = &LGraphicBackend::openRestricted,
    .closeRestricted = &LGraphicBackend::closeRestricted
};

static void initConnector(Backend *bknd, SRMConnector *conn)
{
    if (srmConnectorGetUserData(conn))
       return;

    std::ostringstream desc;
    Output *bkndOutput = new Output();
    SRMDevice *device = srmConnectorGetDevice(conn);
    desc << "[" << device->shortName << "::"<< srmConnectorGetName(conn) << "] "
         << srmConnectorGetModel(conn) << " - "
         << srmConnectorGetManufacturer(conn);
    bkndOutput->description = desc.str();

    LOutput::Params params
    {
        // Callback triggered from the LOutput constructor
        .callback = [=](LOutput *output)
        {
            srmConnectorSetUserData(conn, output);
            bkndOutput->conn = conn;
            bkndOutput->physicalSize.setW(srmConnectorGetmmWidth(conn));
            bkndOutput->physicalSize.setH(srmConnectorGetmmHeight(conn));

            SRMListForeach (modeIt, srmConnectorGetModes(conn))
            {
                SRMConnectorMode *mode = (SRMConnectorMode*)srmListItemGetData(modeIt);
                LOutputMode *outputMode = new LOutputMode(
                    output,
                    LSize(srmConnectorModeGetWidth(mode), srmConnectorModeGetHeight(mode)),
                    srmConnectorModeGetRefreshRate(mode) * 1000,
                    srmConnectorModeIsPreferred(mode),
                    mode);
                srmConnectorModeSetUserData(mode, outputMode);
                bkndOutput->modes.push_back(outputMode);
            }

            output->imp()->updateRect();
            bknd->connectedOutputs.push_back(output);
        },

        // Backend data set to output->imp()->graphicBackendData
        .backendData = bkndOutput
    };

    LFactory::createObject<LOutput>(&params);
}

static void uninitConnector(Backend *bknd, SRMConnector *conn)
{
    LOutput *output = (LOutput*)srmConnectorGetUserData(conn);

    if (!output)
        return;

    LCompositor *compositor = (LCompositor*)srmCoreGetUserData(bknd->core);

    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

    LGraphicBackend::outputDestroyBuffers(bkndOutput->textures);

    while (!bkndOutput->modes.empty())
    {
        LOutputMode *mode = bkndOutput->modes.back();
        srmConnectorModeSetUserData((SRMConnectorMode*)mode->data(), NULL);
        delete mode;
        bkndOutput->modes.pop_back();
    }

    compositor->onAnticipatedObjectDestruction(output);
    LVectorRemoveOne(bknd->connectedOutputs, output);
    delete output;
    delete bkndOutput;
    srmConnectorSetUserData(conn, NULL);
}

static void connectorPluggedEventHandler(SRMListener *listener, SRMConnector *conn)
{
    Backend *bknd = (Backend*)srmListenerGetUserData(listener);
    LCompositor *compositor = (LCompositor*)srmCoreGetUserData(bknd->core);
    initConnector(bknd, conn);
    LOutput *output = (LOutput*)srmConnectorGetUserData(conn);
    compositor->seat()->imp()->backendOutputPlugged(output);
}

static void connectorUnpluggedEventHandler(SRMListener *listener, SRMConnector *conn)
{
    Backend *bknd = (Backend*)srmListenerGetUserData(listener);
    LCompositor *compositor = (LCompositor*)srmCoreGetUserData(bknd->core);

    LOutput *output = (LOutput*)srmConnectorGetUserData(conn);
    compositor->seat()->imp()->backendOutputUnplugged(output);
    compositor->removeOutput(output);
    uninitConnector(bknd, conn);
}

static int monitorEventHandler(Int32, UInt32, void *data)
{
    Backend *bknd = (Backend*)data;
    return srmCoreProcessMonitor(bknd->core, 0);
}

static void initializeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendInitializeGL();
}

static void paintGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendPaintGL();
}

static void resizeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    LGraphicBackend::outputDestroyBuffers(bkndOutput->textures);
    output->imp()->backendResizeGL();
}

static void pageFlipped(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    memcpy(&output->imp()->presentationTime,
           srmConnectorGetPresentationTime(bkndOutput->conn),
           sizeof(output->imp()->presentationTime));
    output->imp()->backendPageFlipped();
}

static void uninitializeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendUninitializeGL();
    srmConnectorSetCursor(connector, NULL);
}

static SRMConnectorInterface connectorInterface =
{
    .initializeGL = &initializeGL,
    .paintGL = &paintGL,
    .pageFlipped = &pageFlipped,
    .resizeGL = &resizeGL,
    .uninitializeGL = &uninitializeGL
};

/* BACKEND API */

UInt32 LGraphicBackend::backendGetId()
{
    return LGraphicBackendDRM;
}

void *LGraphicBackend::backendGetContextHandle()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return bknd->core;
}

bool LGraphicBackend::backendInitialize()
{
    libseatEnabled = compositor()->seat()->imp()->initLibseat();

    Backend *bknd = new Backend();
    compositor()->imp()->graphicBackendData = bknd;
    bknd->core = srmCoreCreate(&srmInterface, compositor());
    SRMVersion *version;
    SRMDevice *allocatorDevice;

    auto formatInVector = [](const std::vector<LDMAFormat> &vec, const LDMAFormat &fmt) -> bool
    {
        return std::find(vec.begin(), vec.end(), fmt) != vec.end();
    };

    if (!bknd->core)
    {
        LLog::fatal("[%s] Failed to create SRM core.", BKND_NAME);
        goto fail;
    }

    version = srmCoreGetVersion(bknd->core);

    if (version->major == 0 && version->minor < 9)
    {
        LLog::fatal("[%s] Using SRM v%d.%d.%d but version >= v0.9.0 is required.", BKND_NAME, version->major, version->minor, version->patch);
        srmCoreDestroy(bknd->core);
        goto fail;
    }

    // Link SRMDevice -> LGPU and create lease globals
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice *)srmListItemGetData(devIt);

        for (auto *gpu : bknd->devices)
        {
            if (gpu->fd() == srmDeviceGetFD(dev))
            {
                srmDeviceSetUserData(dev, gpu);
                gpu->m_data = dev;
                break;
            }
        }
    }

    allocatorDevice = srmCoreGetAllocatorDevice(bknd->core);
    bknd->allocator = (LGPU*)srmDeviceGetUserData(allocatorDevice);
    assert(bknd->allocator != nullptr);

    // Fill DMA formats (LDMAFormat = SRMFormat)
    SRMListForeach (fmtIt, srmCoreGetSharedDMATextureFormats(bknd->core))
    {
        SRMFormat *fmt = (SRMFormat*)srmListItemGetData(fmtIt);
        bknd->dmaFormats.emplace_back(fmt->format, fmt->modifier);
    }

    // Fill scanout DMA formats (requires SRM >= 0.7.0)
    SRMListForeach (planeIt, srmDeviceGetPlanes(allocatorDevice))
    {
        SRMPlane *plane = (SRMPlane*) srmListItemGetData(planeIt);

        if (srmPlaneGetType(plane) != SRM_PLANE_TYPE_PRIMARY)
            continue;

        SRMListForeach (fmtIt, srmPlaneGetFormats(plane))
        {
            SRMFormat *fmt = (SRMFormat*)srmListItemGetData(fmtIt);

            if (!formatInVector(bknd->scanoutFormats, *(LDMAFormat*)fmt) && formatInVector(bknd->dmaFormats, *(LDMAFormat*)fmt))
                bknd->scanoutFormats.emplace_back(*(LDMAFormat*)fmt);

            UInt32 alphaSubstitute = srmFormatGetAlphaSubstitute(fmt->format);

            if (alphaSubstitute != fmt->format
                && !formatInVector(bknd->scanoutFormats, {alphaSubstitute, fmt->modifier})
                && formatInVector(bknd->dmaFormats, {alphaSubstitute, fmt->modifier}))
                bknd->scanoutFormats.emplace_back(alphaSubstitute, fmt->modifier);
        }
    }

    // Find connected outputs
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice*)srmListItemGetData(devIt);

        SRMListForeach (connIt, srmDeviceGetConnectors(dev))
        {
            SRMConnector *conn = (SRMConnector*)srmListItemGetData(connIt);

            if (srmConnectorIsConnected(conn))
                initConnector(bknd, conn);
        }
    }

    // Listen to connector hotplug events
    srmCoreAddConnectorPluggedEventListener(bknd->core, &connectorPluggedEventHandler, bknd);
    srmCoreAddConnectorUnpluggedEventListener(bknd->core, &connectorUnpluggedEventHandler, bknd);

    bknd->monitor = LCompositor::addFdListener(srmCoreGetMonitorFD(bknd->core),
                                            bknd,
                                            &monitorEventHandler);

    compositor()->imp()->graphicBackendData = bknd;
    return true;

    fail:
    delete bknd;
    return false;
}

void LGraphicBackend::backendUninitialize()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    LCompositor::removeFdListener(bknd->monitor);

    // Find connected outputs
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice*)srmListItemGetData(devIt);

        SRMListForeach (connIt, srmDeviceGetConnectors(dev))
        {
            SRMConnector *conn = (SRMConnector*)srmListItemGetData(connIt);
            srmConnectorUninitialize(conn);
            uninitConnector(bknd, conn);
        }
    }

    srmCoreDestroy(bknd->core);
    delete bknd;
}

void LGraphicBackend::backendSuspend()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    srmCoreSuspend(bknd->core);
}

void LGraphicBackend::backendResume()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;

    // All leases should have been destroyed on backendSuspend()
    // but cleanup may have failed if DRM master was lost, let's try now
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice*)srmListItemGetData(devIt);
        drmModeLesseeListRes *lessees { drmModeListLessees(srmDeviceGetFD(dev)) };

        if (!lessees)
            continue;

        for (UInt32 i = 0; i < lessees->count; i++)
        {
            LLog::debug("[%s] Removing previously leaked DRM lease (%d).", BKND_NAME, lessees->lessees[i]);
            drmModeRevokeLease(srmDeviceGetFD(dev), lessees->lessees[i]);
        }

        drmFree(lessees);
    }

    srmCoreResume(bknd->core);
}

const std::vector<LOutput*> *LGraphicBackend::backendGetConnectedOutputs()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return &bknd->connectedOutputs;
}

const std::vector<LGPU*> *LGraphicBackend::backendGetDevices()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return &bknd->devices;
}

const std::vector<LDMAFormat> *LGraphicBackend::backendGetDMAFormats()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return &bknd->dmaFormats;
}

const std::vector<LDMAFormat> *LGraphicBackend::backendGetScanoutDMAFormats()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return &bknd->scanoutFormats;
}

EGLDisplay LGraphicBackend::backendGetAllocatorEGLDisplay()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return srmDeviceGetEGLDisplay(srmCoreGetAllocatorDevice(bknd->core));
}

EGLContext LGraphicBackend::backendGetAllocatorEGLContext()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return srmDeviceGetEGLContext(srmCoreGetAllocatorDevice(bknd->core));
}

LGPU *LGraphicBackend::backendGetAllocatorDevice()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return bknd->allocator;
}

/* TEXTURES */

bool LGraphicBackend::textureCreateFromCPUBuffer(LTexture *texture, const LSize &size, UInt32 stride, UInt32 format, const void *pixels)
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateFromCPU(bknd->core, NULL, size.w(), size.h(), stride, pixels, format);

    if (bkndBuffer)
    {
        texture->m_graphicBackendData = bkndBuffer;
        return true;
    }

    return false;
}

bool LGraphicBackend::textureCreateFromWaylandDRM(LTexture *texture, void *wlBuffer)
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateFromWaylandDRM(bknd->core, wlBuffer);

    if (bkndBuffer)
    {
        texture->m_graphicBackendData = bkndBuffer;
        texture->m_format = srmBufferGetFormat(bkndBuffer);
        texture->m_sizeB.setW(srmBufferGetWidth(bkndBuffer));
        texture->m_sizeB.setH(srmBufferGetHeight(bkndBuffer));
        return true;
    }

    return false;
}

bool LGraphicBackend::textureCreateFromDMA(LTexture *texture, const LDMAPlanes *planes)
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateFromDMA(bknd->core, NULL, (SRMBufferDMAData*)planes);

    if (bkndBuffer)
    {
        texture->m_graphicBackendData = bkndBuffer;
        texture->m_format = srmBufferGetFormat(bkndBuffer);
        texture->m_sizeB.setW(srmBufferGetWidth(bkndBuffer));
        texture->m_sizeB.setH(srmBufferGetHeight(bkndBuffer));
        return true;
    }

    return false;
}

bool LGraphicBackend::textureCreateFromGL(LTexture *texture, GLuint id, GLenum target, UInt32 format, const LSize &size, bool transferOwnership)
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateGLTextureWrapper(
        srmCoreGetAllocatorDevice(bknd->core), id, target, format, size.w(), size.h(), transferOwnership);

    if (bkndBuffer)
    {
        texture->m_graphicBackendData = bkndBuffer;
        texture->m_format = srmBufferGetFormat(bkndBuffer);
        texture->m_sizeB.setW(srmBufferGetWidth(bkndBuffer));
        texture->m_sizeB.setH(srmBufferGetHeight(bkndBuffer));
        return true;
    }

    return false;
}

bool LGraphicBackend::textureUpdateRect(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels)
{
    SRMBuffer *bkndBuffer = (SRMBuffer*)texture->m_graphicBackendData;
    return srmBufferWrite(bkndBuffer, stride, dst.x(), dst.y(), dst.w(), dst.h(), pixels);
}

UInt32 LGraphicBackend::textureGetID(LOutput *output, LTexture *texture)
{
    SRMDevice *bkndRendererDevice;

    if (output)
    {
        Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
        bkndRendererDevice = srmDeviceGetRendererDevice(srmConnectorGetDevice(bkndOutput->conn));
    }
    else
    {
        Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
        bkndRendererDevice = srmCoreGetAllocatorDevice(bknd->core);
    }

    return srmBufferGetTextureID(bkndRendererDevice, (SRMBuffer*)texture->m_graphicBackendData);
}

GLenum LGraphicBackend::textureGetTarget(LTexture *texture)
{
    SRMBuffer *bkndBuffer = (SRMBuffer*)texture->m_graphicBackendData;
    return srmBufferGetTextureTarget(bkndBuffer);
}

void LGraphicBackend::textureSetFence(LTexture *texture)
{
    SRMBuffer *bkndBuffer = (SRMBuffer*)texture->m_graphicBackendData;
    return srmBufferCreateSync(bkndBuffer);
}

void LGraphicBackend::textureDestroy(LTexture *texture)
{
    SRMBuffer *buffer = (SRMBuffer*)texture->m_graphicBackendData;

    if (buffer)
        srmBufferDestroy(buffer);
}

/* OUTPUT */

bool LGraphicBackend::outputInitialize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorInitialize(bkndOutput->conn, &connectorInterface, output);
}

bool LGraphicBackend::outputRepaint(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorRepaint(bkndOutput->conn);
}

void LGraphicBackend::outputUninitialize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    srmConnectorUninitialize(bkndOutput->conn);
    outputDestroyBuffers(bkndOutput->textures);
}

void LGraphicBackend::outputDestroyBuffers(std::vector<LTexture *> &textures)
{
    while (!textures.empty())
    {
        if (textures.back())
        {
            // Do not destroy connectors native buffer
            textures.back()->m_graphicBackendData = nullptr;
            delete textures.back();
        }

        textures.pop_back();
    }
}

bool LGraphicBackend::outputHasBufferDamageSupport(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorHasBufferDamageSupport(bkndOutput->conn);
}

void LGraphicBackend::outputSetBufferDamage(LOutput *output, LRegion &region)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

    if (!srmConnectorHasBufferDamageSupport(bkndOutput->conn) || srmConnectorGetState(bkndOutput->conn) != SRM_CONNECTOR_STATE_INITIALIZED)
        return;

    Int32 n;
    const LBox *boxes = region.boxes(&n);
    srmConnectorSetBufferDamageBoxes(bkndOutput->conn, (SRMBox*)boxes, n);
}

/* OUTPUT PROPS */

const char *LGraphicBackend::outputGetName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetName(bkndOutput->conn);
}

const char *LGraphicBackend::outputGetManufacturerName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetManufacturer(bkndOutput->conn);
}

const char *LGraphicBackend::outputGetModelName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetModel(bkndOutput->conn);
}

const char *LGraphicBackend::outputGetDescription(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return bkndOutput->description.c_str();
}

const char *Louvre::LGraphicBackend::outputGetSerial(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetSerial(bkndOutput->conn);
}

const LSize *LGraphicBackend::outputGetPhysicalSize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return &bkndOutput->physicalSize;
}

Int32 LGraphicBackend::outputGetSubPixel(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return subPixelTable[(Int32)srmConnectorGetSubPixel(bkndOutput->conn)];
}

LGPU *LGraphicBackend::outputGetDevice(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    SRMDevice *dev = srmConnectorGetDevice(bkndOutput->conn);
    LGPU *gpu = (LGPU*)srmDeviceGetUserData(dev);
    assert(gpu != nullptr);
    return gpu;
}

UInt32 LGraphicBackend::outputGetID(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetID(bkndOutput->conn);
}

bool LGraphicBackend::outputIsNonDesktop(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorIsNonDesktop(bkndOutput->conn);
}

/* OUTPUT BUFFERING */

UInt32 LGraphicBackend::outputGetFramebufferID(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetFramebufferID(bkndOutput->conn);
}

Int32 LGraphicBackend::outputGetCurrentBufferIndex(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetCurrentBufferIndex(bkndOutput->conn);
}

UInt32 LGraphicBackend::outputGetBuffersCount(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetBuffersCount(bkndOutput->conn);
}

LTexture *LGraphicBackend::outputGetBuffer(LOutput *output, UInt32 bufferIndex)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

    SRMBuffer *buffer = srmConnectorGetBuffer(bkndOutput->conn, bufferIndex);
    UInt32 buffersCount = srmConnectorGetBuffersCount(bkndOutput->conn);

    if (!buffer || !buffersCount)
        return nullptr;

    if (bkndOutput->textures.empty())
        for (UInt32 i = 0; i < buffersCount; i++)
            bkndOutput->textures.push_back(nullptr);

    if (bkndOutput->textures[bufferIndex])
        return bkndOutput->textures[bufferIndex];

    LTexture *tex = new LTexture(true);
    tex->m_graphicBackendData = buffer;
    tex->m_format = srmBufferGetFormat(buffer);
    tex->m_sizeB.setW(srmBufferGetWidth(buffer));
    tex->m_sizeB.setH(srmBufferGetHeight(buffer));
    bkndOutput->textures[bufferIndex] = tex;
    return tex;
}

/* OUTPUT GAMMA */

UInt32 LGraphicBackend::outputGetGammaSize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetGammaSize(bkndOutput->conn);
}

bool LGraphicBackend::outputSetGamma(LOutput *output, const LGammaTable &table)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

    if (table.size() != srmConnectorGetGammaSize(bkndOutput->conn))
    {
        LLog::error("[%s] Failed to set gamma to output %s. Invalid size %d != real gamma size %d.",
                    BKND_NAME,
                    output->name(),
                    table.size(),
                    output->gammaSize());
        return false;
    }
    return srmConnectorSetGamma(bkndOutput->conn, table.red());
}

/* OUTPUT V-SYNC */
bool LGraphicBackend::outputHasVSyncControlSupport(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorHasVSyncControlSupport(bkndOutput->conn);
}

bool LGraphicBackend::outputIsVSyncEnabled(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorIsVSyncEnabled(bkndOutput->conn);
}

bool LGraphicBackend::outputEnableVSync(LOutput *output, bool enabled)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorEnableVSync(bkndOutput->conn, enabled);
}

void LGraphicBackend::outputSetRefreshRateLimit(LOutput *output, Int32 hz)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    srmConnectorSetRefreshRateLimit(bkndOutput->conn, hz);
}

Int32 LGraphicBackend::outputGetRefreshRateLimit(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetRefreshRateLimit(bkndOutput->conn);
}

/* OUTPUT TIME */

clockid_t LGraphicBackend::outputGetClock(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetPresentationClock(bkndOutput->conn);
}

/* OUTPUT CURSOR */

bool LGraphicBackend::outputHasHardwareCursorSupport(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorHasHardwareCursor(bkndOutput->conn);
}

void LGraphicBackend::outputSetCursorTexture(LOutput *output, UChar8 *buffer)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    srmConnectorSetCursor(bkndOutput->conn, buffer);
}

void LGraphicBackend::outputSetCursorPosition(LOutput *output, const LPoint &position)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    srmConnectorSetCursorPos(bkndOutput->conn, position.x(), position.y());
}

/* OUTPUT MODES */

const LOutputMode *LGraphicBackend::outputGetPreferredMode(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    SRMConnectorMode *mode = srmConnectorGetPreferredMode(bkndOutput->conn);
    return (LOutputMode*)srmConnectorModeGetUserData(mode);
}

const LOutputMode *LGraphicBackend::outputGetCurrentMode(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    SRMConnectorMode *mode = srmConnectorGetCurrentMode(bkndOutput->conn);
    return (LOutputMode*)srmConnectorModeGetUserData(mode);
}

const std::vector<LOutputMode *> *LGraphicBackend::outputGetModes(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return &bkndOutput->modes;
}

bool LGraphicBackend::outputSetMode(LOutput *output, LOutputMode *mode)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorSetMode(bkndOutput->conn, (SRMConnectorMode*)mode->m_data);
}

LContentType LGraphicBackend::outputGetContentType(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return static_cast<LContentType>(srmConnectorGetContentType(bkndOutput->conn) - 1);
}

void LGraphicBackend::outputSetContentType(LOutput *output, LContentType type)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorSetContentType(bkndOutput->conn, (SRM_CONNECTOR_CONTENT_TYPE)(type + 1));
}

/* DIRECT SCANOUT */

bool LGraphicBackend::outputSetScanoutBuffer(LOutput *output, LTexture *texture)
{
    SRMBuffer *buffer { nullptr };

    if (texture)
    {
        /* Framebuffer and Native types are created by Louvre */
        if (!texture->m_graphicBackendData || texture->sourceType() > LTexture::DMA)
            return false;

        buffer = static_cast<SRMBuffer*>(texture->m_graphicBackendData);
    }

    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

    return srmConnectorSetCustomScanoutBuffer(bkndOutput->conn, buffer);
}

/* DRM LEASE */

int LGraphicBackend::backendCreateLease(const std::vector<LOutput*> &outputs)
{
    std::vector<UInt32> resources;
    std::unique_ptr<DRMLease> lease { new DRMLease };
    Output *bkndOutput;
    SRMDevice *dev;

    for (LOutput *output : outputs)
    {
        bkndOutput = (Output*)output->imp()->graphicBackendData;

        // Set GPU
        if (!lease->gpu)
        {
            lease->gpu = output->gpu();
            dev = (SRMDevice*)lease->gpu->m_data;

            // When using the legacy DRM API there is no way to know which cursor and primary plane will a connector use
            if (!srmDeviceGetClientCapAtomic(dev))
            {
                LLog::error("[%s] DRM lease is disabled for the legacy DRM API. Make sure to set SRM_FORCE_LEGACY_API=0.", BKND_NAME);
                return -1;
            }
        }

        // Find and add a free crtc
        SRMEncoder *encoder;
        SRMCrtc *crtc;
        SRMPlane *primaryPlane;
        SRMPlane *cursorPlane;

        if (!srmConnectorGetBestConfiguration(bkndOutput->conn, &encoder, &crtc, &primaryPlane, &cursorPlane))
        {
            LLog::error("[%s] DRM lease: Failed to find a free CRTC for connector %s.", BKND_NAME, output->name());
            return -1;
        }

        // Just mark encoder as used
        lease->encoders.push_back(encoder);
        encoder->currentConnector = bkndOutput->conn;

        // Add crtc
        resources.push_back(crtc->id);
        lease->crtcs.push_back(crtc);
        crtc->currentConnector = bkndOutput->conn;

        // Add primary plane
        resources.push_back(primaryPlane->id);
        lease->planes.push_back(primaryPlane);
        primaryPlane->currentConnector = bkndOutput->conn;

        // Add connector
        resources.push_back(output->id());
        lease->connectors.push_back(bkndOutput->conn);
        bkndOutput->conn->currentCrtc = crtc;
        bkndOutput->conn->currentEncoder = encoder;
        bkndOutput->conn->currentPrimaryPlane = primaryPlane;

        // TODO: Add cursor planes

        // Attach lease to each output
        bkndOutput->lease.reset(lease.get());
    }

    lease->fd = drmModeCreateLease(lease->gpu->fd(), resources.data(), resources.size(), O_CLOEXEC, &lease->lessee);

    if (lease->fd < 0)
    {
        LLog::error("[%s] drmModeCreateLease failed.", BKND_NAME);
        return -1;
    }

    LLog::debug("[%s] New DRM lease (%d).", BKND_NAME, lease->lessee);
    return lease.release()->fd; // Keep pointer alive
}

void LGraphicBackend::backendRevokeLease(int fd)
{
    for (LOutput *output : seat()->outputs())
    {
        Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

        if (bkndOutput->lease && bkndOutput->lease->fd == fd)
        {
            // This also sets to nullptr the lease of other related outputs
            delete bkndOutput->lease.get();
            return;
        }
    }
}

static LGraphicBackendInterface API;

extern "C" LGraphicBackendInterface *getAPI()
{
    API.backendGetId                    = &LGraphicBackend::backendGetId;
    API.backendGetContextHandle         = &LGraphicBackend::backendGetContextHandle;
    API.backendInitialize               = &LGraphicBackend::backendInitialize;
    API.backendUninitialize             = &LGraphicBackend::backendUninitialize;
    API.backendSuspend                  = &LGraphicBackend::backendSuspend;
    API.backendResume                   = &LGraphicBackend::backendResume;
    API.backendGetConnectedOutputs      = &LGraphicBackend::backendGetConnectedOutputs;
    API.backendGetDevices               = &LGraphicBackend::backendGetDevices;
    API.backendGetDMAFormats            = &LGraphicBackend::backendGetDMAFormats;
    API.backendGetScanoutDMAFormats     = &LGraphicBackend::backendGetScanoutDMAFormats;
    API.backendGetAllocatorEGLDisplay   = &LGraphicBackend::backendGetAllocatorEGLDisplay;
    API.backendGetAllocatorEGLContext   = &LGraphicBackend::backendGetAllocatorEGLContext;
    API.backendGetAllocatorDevice       = &LGraphicBackend::backendGetAllocatorDevice;

    /* TEXTURES */
    API.textureCreateFromCPUBuffer      = &LGraphicBackend::textureCreateFromCPUBuffer;
    API.textureCreateFromWaylandDRM     = &LGraphicBackend::textureCreateFromWaylandDRM;
    API.textureCreateFromDMA            = &LGraphicBackend::textureCreateFromDMA;
    API.textureCreateFromGL             = &LGraphicBackend::textureCreateFromGL;
    API.textureUpdateRect               = &LGraphicBackend::textureUpdateRect;
    API.textureGetID                    = &LGraphicBackend::textureGetID;
    API.textureGetTarget                = &LGraphicBackend::textureGetTarget;
    API.textureSetFence                 = &LGraphicBackend::textureSetFence;
    API.textureDestroy                  = &LGraphicBackend::textureDestroy;

    /* OUTPUT */
    API.outputInitialize                = &LGraphicBackend::outputInitialize;
    API.outputRepaint                   = &LGraphicBackend::outputRepaint;
    API.outputUninitialize              = &LGraphicBackend::outputUninitialize;
    API.outputHasBufferDamageSupport    = &LGraphicBackend::outputHasBufferDamageSupport;
    API.outputSetBufferDamage           = &LGraphicBackend::outputSetBufferDamage;

    /* OUTPUT PROPS */
    API.outputGetName                   = &LGraphicBackend::outputGetName;
    API.outputGetManufacturerName       = &LGraphicBackend::outputGetManufacturerName;
    API.outputGetModelName              = &LGraphicBackend::outputGetModelName;
    API.outputGetDescription            = &LGraphicBackend::outputGetDescription;
    API.outputGetSerial                 = &LGraphicBackend::outputGetSerial;
    API.outputGetPhysicalSize           = &LGraphicBackend::outputGetPhysicalSize;
    API.outputGetSubPixel               = &LGraphicBackend::outputGetSubPixel;
    API.outputGetDevice                 = &LGraphicBackend::outputGetDevice;
    API.outputGetID                     = &LGraphicBackend::outputGetID;
    API.outputIsNonDesktop              = &LGraphicBackend::outputIsNonDesktop;

    /* OUTPUT BUFFERING */
    API.outputGetFramebufferID          = &LGraphicBackend::outputGetFramebufferID;
    API.outputGetCurrentBufferIndex     = &LGraphicBackend::outputGetCurrentBufferIndex;
    API.outputGetBuffersCount           = &LGraphicBackend::outputGetBuffersCount;
    API.outputGetBuffer                 = &LGraphicBackend::outputGetBuffer;

    /* OUTPUT GAMMA */
    API.outputGetGammaSize              = &LGraphicBackend::outputGetGammaSize;
    API.outputSetGamma                  = &LGraphicBackend::outputSetGamma;

    /* OUTPUT V-SYNC */
    API.outputHasVSyncControlSupport    = &LGraphicBackend::outputHasVSyncControlSupport;
    API.outputIsVSyncEnabled            = &LGraphicBackend::outputIsVSyncEnabled;
    API.outputEnableVSync               = &LGraphicBackend::outputEnableVSync;
    API.outputSetRefreshRateLimit       = &LGraphicBackend::outputSetRefreshRateLimit;
    API.outputGetRefreshRateLimit       = &LGraphicBackend::outputGetRefreshRateLimit;

    /* OUTPUT TIME */
    API.outputGetClock                  = &LGraphicBackend::outputGetClock;

    /* OUTPUT CURSOR */
    API.outputHasHardwareCursorSupport  = &LGraphicBackend::outputHasHardwareCursorSupport;
    API.outputSetCursorTexture          = &LGraphicBackend::outputSetCursorTexture;
    API.outputSetCursorPosition         = &LGraphicBackend::outputSetCursorPosition;

    /* OUTPUT MODES */
    API.outputGetPreferredMode          = &LGraphicBackend::outputGetPreferredMode;
    API.outputGetCurrentMode            = &LGraphicBackend::outputGetCurrentMode;
    API.outputGetModes                  = &LGraphicBackend::outputGetModes;
    API.outputSetMode                   = &LGraphicBackend::outputSetMode;

    /* CONTENT TYPE */
    API.outputGetContentType            = &LGraphicBackend::outputGetContentType;
    API.outputSetContentType            = &LGraphicBackend::outputSetContentType;

    /* DIRECT SCANOUT */
    API.outputSetScanoutBuffer          = &LGraphicBackend::outputSetScanoutBuffer;

    /* DRM LEASE */
    API.backendCreateLease              = &LGraphicBackend::backendCreateLease;
    API.backendRevokeLease              = &LGraphicBackend::backendRevokeLease;
    return &API;
}
