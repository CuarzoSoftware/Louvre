#include <LLog.h>
#include <sys/epoll.h>

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <drm.h>
#include <drm_fourcc.h>
#include <unordered_map>


#include <LGraphicBackend.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <private/LOutputManagerPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LTexturePrivate.h>

#include <LWayland.h>
#include <LTime.h>

#include <SRM/SRMCore.h>
#include <SRM/SRMDevice.h>
#include <SRM/SRMConnector.h>
#include <SRM/SRMConnectorMode.h>
#include <SRM/SRMBuffer.h>

#include <SRM/SRMList.h>

using namespace Louvre;
using namespace std;

#define BKND_NAME "DRM BACKEND"

struct Backend
{
    SRMCore *core;
    list<LOutput*>connectedOutputs;
    wl_event_source *monitor;
};

struct Output
{
    SRMConnector *conn;
    LSize physicalSize;
    list<LOutputMode*>modes;
};

struct OutputMode
{
    SRMConnectorMode *mode;
    LSize size;
};

static int openRestricted(const char *path, int flags, void *userData)
{
    L_UNUSED(flags);

    LCompositor *compositor = (LCompositor*)userData;

    Int32 fd;
    compositor->seat()->openDevice(path, &fd);

    return fd;
}

static void closeRestricted(int fd, void *userData)
{
    SRM_UNUSED(userData);
    close(fd);
}

static SRMInterface srmInterface =
{
    .openRestricted = &openRestricted,
    .closeRestricted = &closeRestricted
};

static void connectorPluggedEventHandler(SRMListener *listener, SRMConnector *connector)
{
    SRM_UNUSED(listener);

}

static void connectorUnpluggedEventHandler(SRMListener *listener, SRMConnector *connector)
{
    SRM_UNUSED(listener);
    SRM_UNUSED(connector);

    /* The connnector is automatically uninitialized after this event (if connected)
     * so there is no need to call srmConnectorUninitialize() */
}

static int monitorEventHandler(Int32, UInt32, void *data)
{
    Backend *bknd = (Backend*)data;
    return srmCoreProccessMonitor(bknd->core, 0);
}

static void initializeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendInitialized();

    output->imp()->backendBeforePaint();
    output->initializeGL();
    output->imp()->backendAfterPaint();

}

static void paintGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendBeforePaint();
    output->paintGL();
    output->imp()->backendAfterPaint();
}

static void resizeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendBeforePaint();
    output->resizeGL();
    output->imp()->backendAfterPaint();
}

static void pageFlipped(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendPageFlipped();
}

static void uninitializeGL(SRMConnector *connector, void *userData)
{
    SRM_UNUSED(connector);
    LOutput *output = (LOutput*)userData;
    output->imp()->backendBeforePaint();
    output->uninitializeGL();
    output->imp()->backendAfterPaint();
}

static SRMConnectorInterface connectorInterface =
{
    .initializeGL = &initializeGL,
    .paintGL = &paintGL,
    .pageFlipped = &pageFlipped,
    .resizeGL = &resizeGL,
    .uninitializeGL = &uninitializeGL
};

bool LGraphicBackend::initialize(LCompositor *compositor)
{
    Backend *bknd = new Backend();

    bknd->core = srmCoreCreate(&srmInterface, compositor);

    if (!bknd->core)
    {
        LLog::fatal("[%] Failed to create SRM core.", BKND_NAME);
        goto fail;
    }

    // Find connected outputs
    SRMListForeach (devIt, srmCoreGetDevices(bknd->core))
    {
        SRMDevice *dev = (SRMDevice*)srmListItemGetData(devIt);

        SRMListForeach (connIt, srmDeviceGetConnectors(dev))
        {
            SRMConnector *conn = (SRMConnector*)srmListItemGetData(connIt);

            if (srmConnectorIsConnected(conn))
            {
                /* TODO: Free Output and OutputMode structs */

                LOutput *output = compositor->createOutputRequest();
                output->imp()->compositor = compositor;
                srmConnectorSetUserData(conn, output);

                Output *bkndOutput = new Output();
                output->imp()->graphicBackendData = bkndOutput;

                bkndOutput->conn = conn;
                bkndOutput->physicalSize.setW(srmConnectorGetmmWidth(conn));
                bkndOutput->physicalSize.setH(srmConnectorGetmmHeight(conn));

                SRMListForeach (modeIt, srmConnectorGetModes(conn))
                {
                    SRMConnectorMode *mode = (SRMConnectorMode*)srmListItemGetData(modeIt);
                    LOutputMode *outputMode = new LOutputMode(output);
                    srmConnectorModeSetUserData(mode, outputMode);

                    OutputMode *bkndOutputMode = new OutputMode();
                    bkndOutputMode->mode = mode;
                    bkndOutputMode->size.setW(srmConnectorModeGetWidth(mode));
                    bkndOutputMode->size.setH(srmConnectorModeGetHeight(mode));

                    outputMode->imp()->graphicBackendData = bkndOutputMode;
                    bkndOutput->modes.push_back(outputMode);
                }

                bknd->connectedOutputs.push_back(output);
            }
        }
    }

    // Listen to connector hotplug events
    srmCoreAddConnectorPluggedEventListener(bknd->core, &connectorPluggedEventHandler, bknd);
    srmCoreAddConnectorUnpluggedEventListener(bknd->core, &connectorUnpluggedEventHandler, bknd);

    bknd->monitor = LWayland::addFdListener(srmCoreGetMonitorFD(bknd->core),
                                            bknd,
                                            &monitorEventHandler);

    compositor->imp()->graphicBackendData = bknd;
    return true;

    fail:
    delete bknd;
    return false;
}

void LGraphicBackend::uninitialize(LCompositor *compositor)
{
    Backend *bknd = (Backend*)compositor->imp()->graphicBackendData;
    LWayland::removeFdListener(bknd->monitor);
    srmCoreDestroy(bknd->core);
    delete bknd;
}

const list<LOutput*> *LGraphicBackend::getConnectedOutputs(LCompositor *compositor)
{
    Backend *bknd = (Backend*)compositor->imp()->graphicBackendData;
    return &bknd->connectedOutputs;
}

bool LGraphicBackend::initializeOutput(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorInitialize(bkndOutput->conn, &connectorInterface, output);
}

bool LGraphicBackend::scheduleOutputRepaint(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorRepaint(bkndOutput->conn);
}

void LGraphicBackend::uninitializeOutput(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    srmConnectorUninitialize(bkndOutput->conn);
}

const LSize *LGraphicBackend::getOutputPhysicalSize(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return &bkndOutput->physicalSize;
}

Int32 LGraphicBackend::getOutputCurrentBufferIndex(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetCurrentBufferIndex(bkndOutput->conn);
}

const char *LGraphicBackend::getOutputName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetName(bkndOutput->conn);
}

const char *LGraphicBackend::getOutputManufacturerName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetManufacturer(bkndOutput->conn);
}

const char *LGraphicBackend::getOutputModelName(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorGetModel(bkndOutput->conn);
}

const char *LGraphicBackend::getOutputDescription(LOutput *output)
{
    L_UNUSED(output);
    return "DRM connector.";
}

const LOutputMode *LGraphicBackend::getOutputPreferredMode(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    SRMConnectorMode *mode = srmConnectorGetPreferredMode(bkndOutput->conn);
    return (LOutputMode*)srmConnectorModeGetUserData(mode);
}

const LOutputMode *LGraphicBackend::getOutputCurrentMode(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    SRMConnectorMode *mode = srmConnectorGetCurrentMode(bkndOutput->conn);
    return (LOutputMode*)srmConnectorModeGetUserData(mode);
}

const std::list<LOutputMode *> *LGraphicBackend::getOutputModes(LOutput *output)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return &bkndOutput->modes;
}

bool LGraphicBackend::setOutputMode(LOutput *output, LOutputMode *mode)
{
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    OutputMode *bkndOutputMode = (OutputMode*)mode->imp()->graphicBackendData;
    return srmConnectorSetMode(bkndOutput->conn, bkndOutputMode->mode);
}

const LSize *LGraphicBackend::getOutputModeSize(LOutputMode *mode)
{
    OutputMode *bkndOutputMode = (OutputMode*)mode->imp()->graphicBackendData;
    return &bkndOutputMode->size;
}

Int32 LGraphicBackend::getOutputModeRefreshRate(LOutputMode *mode)
{
    OutputMode *bkndOutputMode = (OutputMode*)mode->imp()->graphicBackendData;
    return srmConnectorModeGetRefreshRate(bkndOutputMode->mode)*1000;
}

bool LGraphicBackend::getOutputModeIsPreferred(LOutputMode *mode)
{
    OutputMode *bkndOutputMode = (OutputMode*)mode->imp()->graphicBackendData;
    return srmConnectorModeIsPreferred(bkndOutputMode->mode);
}

bool LGraphicBackend::hasHardwareCursorSupport(LOutput *output)
{
    return false;
    Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
    return srmConnectorHasHardwareCursor(bkndOutput->conn);
}

void LGraphicBackend::setCursorTexture(LOutput *output, LTexture *texture, LSizeF &size)
{
    /*
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    if(!texture)
    {
        drmModeSetCursor(data->drm.fds.fd, data->drm.crtc_id, 0, 0, 0);
        data->cursor.visible = false;
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, data->cursor.fb);

    output->painter()->imp()->scaleCursor(
                (LTexture*)texture,
                LRect(0,0,texture->sizeB().w(),-texture->sizeB().h()),
                LRect(0,size));

    // Si se invoca desde el hilo principal debemos llamar glFlush para sincronizar el cambio
    if(std::this_thread::get_id() == output->compositor()->mainThreadId())
        glFinish();

    if(data->cursor.isDumb)
    {
        glReadPixels(0, 0, 64, 64, GL_RGBA ,GL_UNSIGNED_BYTE, data->cursor.buffer);


        if(!data->cursor.visible)
            drmModeSetCursor(data->drm.fds.fd, data->drm.crtc_id,
                             data->cursor.handle,
                             64, 64);
    }
    else
    {
        if(!data->cursor.visible)
            drmModeSetCursor(data->drm.fds.fd, data->drm.crtc_id,
                             gbm_bo_get_handle(data->cursor.bo).u32,
                             64, 64);
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
*/

}

void LGraphicBackend::setCursorPosition(LOutput *output, LPoint &position)
{
    /*
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    drmModeMoveCursor(data->drm.fds.fd,
                      data->drm.crtc_id,
                      position.x(),
                      position.y());
*/
}

EGLDisplay LGraphicBackend::getAllocatorEGLDisplay(LCompositor *compositor)
{
    Backend *bknd = (Backend*)compositor->imp()->graphicBackendData;
    return srmDeviceGetEGLDisplay(srmCoreGetAllocatorDevice(bknd->core));
}

bool LGraphicBackend::createTextureFromCPUBuffer(LTexture *texture, const LSize &size, UInt32 stride, UInt32 format, const void *pixels)
{
    Backend *bknd = (Backend*)texture->compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateFromCPU(bknd->core, size.w(), size.h(), stride, pixels, format);

    if (bkndBuffer)
    {
        texture->imp()->graphicBackendData = bkndBuffer;
        return true;
    }

    return false;
}

bool LGraphicBackend::createTextureFromWaylandDRM(LTexture *texture, void *wlBuffer)
{
    Backend *bknd = (Backend*)texture->compositor()->imp()->graphicBackendData;
    SRMBuffer *bkndBuffer = srmBufferCreateFromWaylandDRM(bknd->core, wlBuffer);

    if (bkndBuffer)
    {
        texture->imp()->graphicBackendData = bkndBuffer;
        texture->imp()->format = srmBufferGetFormat(bkndBuffer);
        texture->imp()->sizeB.setW(srmBufferGetWidth(bkndBuffer));
        texture->imp()->sizeB.setH(srmBufferGetHeight(bkndBuffer));

        return true;
    }

    return false;
}

bool LGraphicBackend::updateTextureRect(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels)
{
    SRMBuffer *bkndBuffer = (SRMBuffer*)texture->imp()->graphicBackendData;
    return srmBufferWrite(bkndBuffer, stride, dst.x(), dst.y(), dst.w(), dst.h(), pixels);
}

UInt32 LGraphicBackend::getTextureID(LOutput *output, LTexture *texture)
{
    SRMDevice *bkndRendererDevice;

    if (output)
    {
        Output *bkndOutput = (Output*)output->imp()->graphicBackendData;
        bkndRendererDevice = srmDeviceGetRendererDevice(srmConnectorGetDevice(bkndOutput->conn));
    }
    else
    {
        Backend *bknd = (Backend*)texture->compositor()->imp()->graphicBackendData;
        bkndRendererDevice = srmCoreGetAllocatorDevice(bknd->core);
    }

    return srmBufferGetTextureID(bkndRendererDevice, (SRMBuffer*)texture->imp()->graphicBackendData);

}

void LGraphicBackend::destroyTexture(LTexture *texture)
{
    SRMBuffer *buffer = (SRMBuffer*)texture->imp()->graphicBackendData;
    srmBufferDestroy(buffer);
}

LGraphicBackendInterface API;

extern "C" LGraphicBackendInterface *getAPI()
{
    API.initialize = &LGraphicBackend::initialize;
    API.scheduleOutputRepaint = &LGraphicBackend::scheduleOutputRepaint;
    API.uninitialize = &LGraphicBackend::uninitialize;
    API.getConnectedOutputs = &LGraphicBackend::getConnectedOutputs;
    API.initializeOutput = &LGraphicBackend::initializeOutput;
    API.uninitializeOutput = &LGraphicBackend::uninitializeOutput;
    API.getOutputPhysicalSize = &LGraphicBackend::getOutputPhysicalSize;
    API.getOutputCurrentBufferIndex = &LGraphicBackend::getOutputCurrentBufferIndex;
    API.getOutputName = &LGraphicBackend::getOutputName;
    API.getOutputManufacturerName = &LGraphicBackend::getOutputManufacturerName;
    API.getOutputModelName = &LGraphicBackend::getOutputModelName;
    API.getOutputDescription = &LGraphicBackend::getOutputDescription;
    API.getOutputPreferredMode = &LGraphicBackend::getOutputPreferredMode;
    API.getOutputCurrentMode = &LGraphicBackend::getOutputCurrentMode;
    API.getOutputModes = &LGraphicBackend::getOutputModes;
    API.setOutputMode = &LGraphicBackend::setOutputMode;
    API.getOutputModeSize = &LGraphicBackend::getOutputModeSize;
    API.getOutputModeRefreshRate = &LGraphicBackend::getOutputModeRefreshRate;
    API.getOutputModeIsPreferred = &LGraphicBackend::getOutputModeIsPreferred;
    API.hasHardwareCursorSupport = &LGraphicBackend::hasHardwareCursorSupport;
    API.setCursorTexture = &LGraphicBackend::setCursorTexture;
    API.setCursorPosition = &LGraphicBackend::setCursorPosition;

    // Buffers
    API.getAllocatorEGLDisplay = &LGraphicBackend::getAllocatorEGLDisplay;
    API.createTextureFromCPUBuffer = &LGraphicBackend::createTextureFromCPUBuffer;
    API.createTextureFromWaylandDRM = &LGraphicBackend::createTextureFromWaylandDRM;
    API.updateTextureRect = &LGraphicBackend::updateTextureRect;
    API.getTextureID = &LGraphicBackend::getTextureID;
    API.destroyTexture = &LGraphicBackend::destroyTexture;

    return &API;
}

