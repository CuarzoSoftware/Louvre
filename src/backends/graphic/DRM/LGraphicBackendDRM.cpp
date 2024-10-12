#include <LLog.h>
#include <sys/epoll.h>

#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

#include <LTime.h>
#include <LGammaTable.h>
#include <LOutputMode.h>

#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMBuffer.h>
#include <SRMListener.h>
#include <SRMList.h>
#include <SRMFormat.h>
#include <SRMPlane.h>

using namespace Louvre;

#define SRM_VERSION_GREATER_EQUAL_THAN(major, minor, patch) (major < SRM_VERSION_MAJOR || (major == SRM_VERSION_MAJOR && (minor < SRM_VERSION_MINOR || (minor == SRM_VERSION_MINOR && patch <= SRM_VERSION_PATCH))))
#define SRM_VERSION_LESS_THAN(major, minor, patch) !SRM_VERSION_GREATER_EQUAL_THAN(major, minor, patch)

#define BKND_NAME "DRM BACKEND"

static bool libseatEnabled { false };

struct DEVICE_FD_ID
{
    int fd;
    int id;
};

struct Backend
{
    SRMCore *core;
    std::vector<LOutput*>connectedOutputs;
    wl_event_source *monitor;
    std::vector<LDMAFormat>dmaFormats;
    std::vector<LDMAFormat>scanoutFormats;
    std::list<DEVICE_FD_ID> devices;
    UInt32 rendererGPUs {0};
    dev_t allocatorDeviceId;
};

struct Output
{
    SRMConnector *conn;
    LSize physicalSize;
    std::vector<LOutputMode*>modes;
    std::vector<LTexture*> textures;

#if SRM_VERSION_LESS_THAN(0,6,0)
    LContentType fallbackContentType { LContentTypeNone };
#endif
};

// SRM -> Louvre Subpixel
#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
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
#endif

static int openRestricted(const char *path, int flags, void *userData)
{
    LCompositor *compositor {(LCompositor*)userData};
    Backend *bknd {(Backend*)compositor->imp()->graphicBackendData};

    if (libseatEnabled)
    {
        DEVICE_FD_ID dev;

        dev.id = compositor->seat()->openDevice(path, &dev.fd);

        if (dev.id == -1)
            return -1;
        else
        {
            bknd->devices.push_back(dev);
            return dev.fd;
        }
    }
    else
        return open(path, flags);
}

static void closeRestricted(int fd, void *userData)
{
    LCompositor *compositor {(LCompositor*)userData};
    Backend *bknd {(Backend*)compositor->imp()->graphicBackendData};

    if (libseatEnabled)
    {
        DEVICE_FD_ID dev {-1, -1};

        for (std::list<DEVICE_FD_ID>::iterator it = bknd->devices.begin(); it != bknd->devices.end(); it++)
        {
            if ((*it).fd == fd)
            {
                dev = (*it);
                bknd->devices.erase(it);
                break;
            }
        }

        if (dev.fd == -1)
            return;

        compositor->seat()->closeDevice(dev.id);
    }

    close(fd);
}

static SRMInterface srmInterface =
{
    .openRestricted = &openRestricted,
    .closeRestricted = &closeRestricted
};

static void initConnector(Backend *bknd, SRMConnector *conn)
{
    if (srmConnectorGetUserData(conn))
       return;

    Output *bkndOutput = new Output();

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

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    memcpy(&output->imp()->presentationTime,
           srmConnectorGetPresentationTime(bkndOutput->conn),
           sizeof(output->imp()->presentationTime));
#else
    output->imp()->presentationTime.flags = SRM_PRESENTATION_TIME_FLAGS_VSYNC;
    output->imp()->presentationTime.frame = 0;
    output->imp()->presentationTime.period = 0;
    clock_gettime(CLOCK_MONOTONIC, &output->imp()->presentationTime.time);
#endif

    output->imp()->backendPageFlipped();
}

static void uninitializeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendUninitializeGL();
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

    struct stat stat;
    Backend *bknd = new Backend();
    compositor()->imp()->graphicBackendData = bknd;
    bknd->core = srmCoreCreate(&srmInterface, compositor());
    SRMVersion *version;
    SRMDevice *allocatorDevice;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,7,0)
    auto formatInVector = [](const std::vector<LDMAFormat> &vec, const LDMAFormat &fmt) -> bool
    {
        return std::find(vec.begin(), vec.end(), fmt) != vec.end();
    };
#endif

    if (!bknd->core)
    {
        LLog::fatal("[%s] Failed to create SRM core.", BKND_NAME);
        goto fail;
    }

    version = srmCoreGetVersion(bknd->core);

    if (version->major == 0 && version->minor == 5 && version->patch == 1)
    {
        LLog::fatal("[%s] You are currently using SRM v0.5.1, which has serious bugs causing issues with the refresh rate and hardware cursor plane updates. Consider upgrading to v0.5.2 or a later version.", BKND_NAME);
        srmCoreDestroy(bknd->core);
        goto fail;
    }

    allocatorDevice = srmCoreGetAllocatorDevice(bknd->core);

    if (fstat(srmDeviceGetFD(allocatorDevice), &stat) == 0)
        bknd->allocatorDeviceId = stat.st_rdev;
    else
    {
        bknd->allocatorDeviceId = -1;
        LLog::fatal("[%s] Failed to get allocator device ID.", BKND_NAME);
    }

    // Fill DMA formats (LDMAFormat = SRMFormat)
    SRMListForeach (fmtIt, srmCoreGetSharedDMATextureFormats(bknd->core))
    {
        SRMFormat *fmt = (SRMFormat*)srmListItemGetData(fmtIt);
        bknd->dmaFormats.emplace_back(fmt->format, fmt->modifier);
    }

    // Fill scanout DMA formats (requires SRM >= 0.7.0)
#if SRM_VERSION_GREATER_EQUAL_THAN(0,7,0)

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
#endif

    // Find connected outputs
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice*)srmListItemGetData(devIt);

        if (srmDeviceIsRenderer(dev))
            bknd->rendererGPUs++;

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
    srmCoreResume(bknd->core);
}

const std::vector<LOutput*> *LGraphicBackend::backendGetConnectedOutputs()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return &bknd->connectedOutputs;
}

UInt32 LGraphicBackend::backendGetRendererGPUs()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return bknd->rendererGPUs;
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

dev_t LGraphicBackend::backendGetAllocatorDeviceId()
{
    Backend *bknd = (Backend*)compositor()->imp()->graphicBackendData;
    return bknd->allocatorDeviceId;
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

// Boxes are supported since SRM v0.5.0
#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    srmConnectorSetBufferDamageBoxes(bkndOutput->conn, (SRMBox*)boxes, n);
#else
    SRMRect rects[n];

    for (Int32 i = 0; i < n; i++)
    {
        rects[i].x = boxes[i].x1;
        rects[i].y = boxes[i].y1;
        rects[i].width = boxes[i].x2 - boxes[i].x1;
        rects[i].height = boxes[i].y2 - boxes[i].y1;
    }
    srmConnectorSetBufferDamage(bkndOutput->conn, rects, n);
#endif
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
    L_UNUSED(output);
    return "DRM connector";
}

const LSize *LGraphicBackend::outputGetPhysicalSize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return &bkndOutput->physicalSize;
}

Int32 LGraphicBackend::outputGetSubPixel(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return subPixelTable[(Int32)srmConnectorGetSubPixel(bkndOutput->conn)];
#else
    return WL_OUTPUT_SUBPIXEL_UNKNOWN;
#endif
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

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorGetGammaSize(bkndOutput->conn);
#else
    L_UNUSED(bkndOutput);
    return 0;
#endif
}

bool LGraphicBackend::outputSetGamma(LOutput *output, const LGammaTable &table)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
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
#else
    L_UNUSED(bkndOutput)
    return false;
#endif
}

/* OUTPUT V-SYNC */
bool LGraphicBackend::outputHasVSyncControlSupport(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorHasVSyncControlSupport(bkndOutput->conn);
#else
    return false;
#endif
}

bool LGraphicBackend::outputIsVSyncEnabled(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorIsVSyncEnabled(bkndOutput->conn);
#else
    return true;
#endif
}

bool LGraphicBackend::outputEnableVSync(LOutput *output, bool enabled)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorEnableVSync(bkndOutput->conn, enabled);
#else
    return enabled;
#endif
}

void LGraphicBackend::outputSetRefreshRateLimit(LOutput *output, Int32 hz)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    srmConnectorSetRefreshRateLimit(bkndOutput->conn, hz);
#else
    L_UNUSED(bkndOutput)
#endif
}

Int32 LGraphicBackend::outputGetRefreshRateLimit(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorGetRefreshRateLimit(bkndOutput->conn);
#else
    return 0;
#endif
}

/* OUTPUT TIME */

clockid_t LGraphicBackend::outputGetClock(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_GREATER_EQUAL_THAN(0,5,0)
    return srmConnectorGetPresentationClock(bkndOutput->conn);
#else
    return CLOCK_MONOTONIC;
#endif
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

#if SRM_VERSION_LESS_THAN(0,6,0)
    return bkndOutput->fallbackContentType;
#else
    return static_cast<LContentType>(srmConnectorGetContentType(bkndOutput->conn) - 1);
#endif
}

void LGraphicBackend::outputSetContentType(LOutput *output, LContentType type)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;

#if SRM_VERSION_LESS_THAN(0,6,0)
    bkndOutput->fallbackContentType = type;
#else
    return srmConnectorSetContentType(bkndOutput->conn, (SRM_CONNECTOR_CONTENT_TYPE)(type + 1));
#endif
}

/* DIRECT SCANOUT */
bool LGraphicBackend::outputSetScanoutBuffer(LOutput *output, LTexture *texture)
{
#if SRM_VERSION_GREATER_EQUAL_THAN(0,7,0)
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
#else
    L_UNUSED(output)
    L_UNUSED(texture)
    LLog::warning("[%s] Direct scanout requires SRM >= 0.7.0", BKND_NAME);
    return false;
#endif
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
    API.backendGetRendererGPUs          = &LGraphicBackend::backendGetRendererGPUs;
    API.backendGetDMAFormats            = &LGraphicBackend::backendGetDMAFormats;
    API.backendGetScanoutDMAFormats     = &LGraphicBackend::backendGetScanoutDMAFormats;
    API.backendGetAllocatorEGLDisplay   = &LGraphicBackend::backendGetAllocatorEGLDisplay;
    API.backendGetAllocatorEGLContext   = &LGraphicBackend::backendGetAllocatorEGLContext;
    API.backendGetAllocatorDeviceId     = &LGraphicBackend::backendGetAllocatorDeviceId;

    /* TEXTURES */
    API.textureCreateFromCPUBuffer      = &LGraphicBackend::textureCreateFromCPUBuffer;
    API.textureCreateFromWaylandDRM     = &LGraphicBackend::textureCreateFromWaylandDRM;
    API.textureCreateFromDMA            = &LGraphicBackend::textureCreateFromDMA;
    API.textureUpdateRect               = &LGraphicBackend::textureUpdateRect;
    API.textureGetID                    = &LGraphicBackend::textureGetID;
    API.textureGetTarget                = &LGraphicBackend::textureGetTarget;
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
    API.outputGetPhysicalSize           = &LGraphicBackend::outputGetPhysicalSize;
    API.outputGetSubPixel               = &LGraphicBackend::outputGetSubPixel;

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

    return &API;
}
