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

#include <LWayland.h>
#include <LTime.h>


#include <LDRM.h>

LDRM *drm = nullptr;

#define WEAK __attribute__((weak))

using namespace Louvre;

std::unordered_map<int,int>devicesID;

// Lista de salidas disponibles
std::list<LOutput*>outputs;

// Mutex para impedir que múltiples salidas llamen al método drmHandleEvent simultaneamente
mutex pageFlipMutex;

struct GL_DATA
{
    EGLDisplay display;
    EGLConfig config;
    EGLContext context = NULL;
    EGLSurface surface;
};

struct GBM_DATA
{
    gbm_device *dev;
    gbm_surface *surface;
};

struct CURSOR_DATA
{
    bool initialized = false;
    bool visible = false;
    gbm_bo *bo = nullptr;
    EGLImage eglImage;
    GLuint texture;
    GLuint fb = 0;

    // Dumb buffer
    bool isDumb;
    UChar8 *buffer;
    int fd;
    UInt32 handle;
    UInt32 fbId;
};

struct drm_fb
{
    gbm_bo *bo;
    uint32_t fb_id;
};

struct DRM_DATA
{
    pollfd fds;
    char name[64];
    bool pendingPageFlip = false;
    UInt32 crtc_id;
    drmModeConnector *connector = nullptr;
    drmEventContext evctx = {};
};

struct OUTPUT_MODE
{
    drmModeModeInfo *mode;
    LSize size;
    bool isPreferred = false;
};

struct OUTPUT_DATA
{
    Int32 backendId = 1;
    Int32 currentBufferIndex = 0;
    LSize physicalSize;
    list<LOutputMode*>modes;
    LOutputMode *preferredMode;
    LOutputMode *currentMode;

    bool initialized = false;
    bool plugged = false;

    gbm_bo *bo = nullptr;
    drm_fb *fb = nullptr;

    DRM_DATA drm;
    GBM_DATA gbm;
    GL_DATA gl;
    CURSOR_DATA cursor;
    LOutput *output;
};

struct FB_DATA
{
    drm_fb *fb;
    OUTPUT_DATA *data;
};

struct DRM_PLANE
{
    UInt64 type; // (Primary, Overlay, Cursor)
};

struct LDevice
{
    LCompositor *compositor;
    int fd;
    struct udev *udev;
    udev_monitor *monitor;
    udev_device *dev = nullptr;
}lDevice;

int ret;

WEAK struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
                                  uint32_t width, uint32_t height,
                                  uint32_t format,
                                  const uint64_t *modifiers,
                                  const unsigned int count);

WEAK uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo);

WEAK int
gbm_bo_get_plane_count(struct gbm_bo *bo);

WEAK uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane);

WEAK uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane);

static void drm_fb_destroy_callback(gbm_bo *bo, void *data)
{
    (void)bo;

    FB_DATA *fb_data = (FB_DATA*)data;

    drm_fb *fb = fb_data->fb;

    if (fb->fb_id)
        drmModeRmFB(fb_data->data->drm.fds.fd, fb->fb_id);

    free(fb);
    free(fb_data);
}

static struct FB_DATA * drm_fb_get_from_bo(gbm_bo *bo, OUTPUT_DATA *data)
{
    FB_DATA *fb_data = (FB_DATA*)gbm_bo_get_user_data(bo);

    if(!fb_data)
    {
        fb_data = (FB_DATA*)calloc(1, sizeof *fb_data);
        fb_data->data = data;
        fb_data->fb = nullptr;
    }

    if (fb_data->fb)
        return fb_data;

    uint32_t width, height, format, strides[4] = {0}, handles[4] = {0}, offsets[4] = {0}, flags = 0;
    int ret = -1;


    fb_data->fb = (drm_fb*)calloc(1, sizeof(drm_fb));
    fb_data->fb->bo = bo;

    width = gbm_bo_get_width(bo);
    height = gbm_bo_get_height(bo);
    format = gbm_bo_get_format(bo);

    int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));

    if(gbm_bo_get_modifier && gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane && gbm_bo_get_offset)
    {
        uint64_t modifiers[4] = {0};
        modifiers[0] = gbm_bo_get_modifier(bo);

        const int num_planes = gbm_bo_get_plane_count(bo);
        for (int i = 0; i < num_planes; i++)
        {
            strides[i] = gbm_bo_get_stride_for_plane(bo, i);
            handles[i] = gbm_bo_get_handle(bo).u32;
            offsets[i] = gbm_bo_get_offset(bo, i);
            modifiers[i] = modifiers[0];
        }

        if (modifiers[0])
        {
            flags = DRM_MODE_FB_MODIFIERS;
            //printf("Using modifier %" PRIx64 "\n", modifiers[0]);
        }

        ret = drmModeAddFB2WithModifiers(drm_fd, width, height, format, handles, strides, offsets, modifiers, &fb_data->fb->fb_id, flags);
    }

    if(ret)
    {
        if(flags)
            fprintf(stderr, "Modifiers failed!\n");

        uint32_t arr[4];
        arr[0] = gbm_bo_get_handle(bo).u32;
        arr[1] = 0;
        arr[2] = 0;
        arr[3] = 0;
        memcpy(handles, arr, 16);
        arr[0] = gbm_bo_get_stride(bo);
        memcpy(strides, arr, 16);
        memset(offsets, 0, 16);
        ret = drmModeAddFB2(drm_fd, width, height, format, handles, strides, offsets, &fb_data->fb->fb_id, 0);
    }

    if (ret)
    {
        //printf("failed to create fb: %s\n", strerror(errno));
        free(fb_data->fb);
        fb_data->fb = NULL;
        return NULL;
    }

    gbm_bo_set_user_data(bo, fb_data, drm_fb_destroy_callback);

    return fb_data;

}

static uint32_t find_crtc_for_encoder(const drmModeRes *resources, const drmModeEncoder *encoder)
{
    for (int i = 0; i < resources->count_crtcs; i++)
    {
        const uint32_t crtc_mask = 1 << i;
        const uint32_t crtc_id = resources->crtcs[i];

        bool encoderIsFree = true;


        for(LOutput *output : outputs)
        {
            OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
            if(crtc_id == data->drm.crtc_id)
            {
                encoderIsFree = false;
                break;
            }
        }

        if (encoder->possible_crtcs & crtc_mask && encoderIsFree)
            return crtc_id;
    }
    return -1;
}

static uint32_t find_crtc_for_connector(int fd, const drmModeRes *resources, const drmModeConnector *connector)
{
    for (int i = 0; i < connector->count_encoders; i++)
    {
        const uint32_t encoder_id = connector->encoders[i];
        drmModeEncoder *encoder = drmModeGetEncoder(fd, encoder_id);
        if (encoder)
        {
            const uint32_t crtc_id = find_crtc_for_encoder(resources, encoder);

            drmModeFreeEncoder(encoder);
            if (crtc_id != 0)
                return crtc_id;
        }
    }
    return -1;
}

// Se invoca cuando un conector deja de estar conectado, por lo tanto no solo se deinicializa, también se elimina
void destroyOutput(LOutput *output)
{
    // Notifica que la salida será destruida
    output->compositor()->destroyOutputRequest(output);

    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    drmModeFreeConnector(data->drm.connector);

    while(!data->modes.empty())
    {
        LOutputMode *mode = data->modes.back();
        OUTPUT_MODE *modeData = (OUTPUT_MODE*)mode->imp()->graphicBackendData;

        delete modeData;
        delete mode;
        data->modes.pop_back();
    }

    delete data;
    delete output;
}

static const char *conn_name(uint32_t type)
{
    switch (type) {
    case DRM_MODE_CONNECTOR_Unknown:     return "unknown";
    case DRM_MODE_CONNECTOR_VGA:         return "VGA";
    case DRM_MODE_CONNECTOR_DVII:        return "DVI-I";
    case DRM_MODE_CONNECTOR_DVID:        return "DVI-D";
    case DRM_MODE_CONNECTOR_DVIA:        return "DVI-A";
    case DRM_MODE_CONNECTOR_Composite:   return "composite";
    case DRM_MODE_CONNECTOR_SVIDEO:      return "S-VIDEO";
    case DRM_MODE_CONNECTOR_LVDS:        return "LVDS";
    case DRM_MODE_CONNECTOR_Component:   return "component";
    case DRM_MODE_CONNECTOR_9PinDIN:     return "DIN";
    case DRM_MODE_CONNECTOR_DisplayPort: return "DisplayPort";
    case DRM_MODE_CONNECTOR_HDMIA:       return "HDMI-A";
    case DRM_MODE_CONNECTOR_HDMIB:       return "HDMI-B";
    case DRM_MODE_CONNECTOR_TV:          return "TV";
    case DRM_MODE_CONNECTOR_eDP:         return "eDP";
    case DRM_MODE_CONNECTOR_VIRTUAL:     return "virtual";
    case DRM_MODE_CONNECTOR_DSI:         return "DSI";
    case DRM_MODE_CONNECTOR_DPI:         return "DPI";
    case DRM_MODE_CONNECTOR_WRITEBACK:   return "writeback";
    default:                             return "unknown";
    }
}

void manageOutputs(bool notify)
{

    drmModeRes *resources;
    drmModeConnector *connector = NULL;
    drmModeEncoder *encoder = NULL;
    uint32_t crtc_id;
    int area;

    resources = drmModeGetResources(lDevice.fd);

    // Iteramos todas las salidas
    for (int i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(lDevice.fd, resources->connectors[i]);

        // Verificamos que esté conectada
        if (connector->connection == DRM_MODE_CONNECTED && connector->mmHeight != 0 && connector->mmWidth != 0)
        {

            bool wasConnected = false;

            // Verificamos si ya se había añadido a la lista de salidas conectadas
            for(LOutput *output : outputs)
            {
                OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
                if(data->drm.connector->connector_id == connector->connector_id)
                {
                    wasConnected = true;
                    break;
                }
            }

            // Si ya estaba añadida la saltamos
            if(wasConnected)
            {
                drmModeFreeConnector(connector);
                continue;
            }

            // Verificamos que tenga al menos un modo o la omitimos
            if (connector->count_modes == 0)
            {
                drmModeFreeConnector(connector);
                continue;
            }

            // Buscamos un encoder disponible
            for (int j = 0; j < resources->count_encoders; j++)
            {
                encoder = drmModeGetEncoder(lDevice.fd, resources->encoders[j]);

                bool encoderIsFree = true;

                for(LOutput *output : outputs)
                {
                    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
                    if(encoder->crtc_id == data->drm.crtc_id)
                    {
                        encoderIsFree = false;
                        break;
                    }
                }

                if(encoderIsFree && encoder->encoder_id == connector->encoder_id)
                {
                    break;
                }
                drmModeFreeEncoder(encoder);
                encoder = NULL;
            }


            if(encoder)
            {
                crtc_id = encoder->crtc_id;
            }
            else
            {
                uint32_t _crtc_id = find_crtc_for_connector(lDevice.fd,resources, connector);

                if (_crtc_id == 0)
                {
                    printf("No crtc found!\n");
                    drmModeFreeConnector(connector);
                    exit(1);
                    continue;
                }

                crtc_id = _crtc_id;
            }

            drmModeFreeEncoder(encoder);


            // Si no estaba añadida creamos una nueva salida
            LOutput *newOutput = lDevice.compositor->createOutputRequest();
            newOutput->imp()->compositor = lDevice.compositor;

            OUTPUT_DATA *newOutputData = new OUTPUT_DATA;
            newOutputData->plugged = true;
            newOutput->imp()->graphicBackendData = newOutputData;
            newOutputData->preferredMode = nullptr;

            // Creamos sus modos
            for (int j = 0; j < connector->count_modes; j++)
            {
                drmModeModeInfo *mode = &connector->modes[j];

                LOutputMode *newMode = new LOutputMode(newOutput);
                newOutputData->modes.push_back(newMode);

                OUTPUT_MODE *newModeData = new OUTPUT_MODE;
                newMode->imp()->graphicBackendData = newModeData;

                newModeData->mode = mode;
                newModeData->size.setW(mode->hdisplay);
                newModeData->size.setH(mode->vdisplay);
                newModeData->isPreferred = false;


            }

            // Obtenemos el modo por defecto
            area = 0;
            for (LOutputMode *outputMode : newOutputData->modes)
            {

                OUTPUT_MODE *newModeData = (OUTPUT_MODE*)outputMode->imp()->graphicBackendData;

                if (newModeData->mode->type & DRM_MODE_TYPE_PREFERRED)
                {
                    newOutputData->currentMode = outputMode;
                    newOutputData->preferredMode = outputMode;
                    newModeData->isPreferred = true;
                    break;
                }

                int current_area = newModeData->size.area();

                if (current_area > area)
                {
                    newOutputData->currentMode = outputMode;
                    newOutputData->preferredMode = outputMode;
                    newModeData->isPreferred = true;
                }
            }

            // RECORDAR ! Eliminar connector y encoder

            newOutputData->output = newOutput;

            newOutputData->physicalSize.setW(connector->mmWidth);
            newOutputData->physicalSize.setH(connector->mmHeight);

            newOutputData->drm.connector = connector;
            newOutputData->drm.crtc_id = crtc_id;
            newOutputData->drm.fds.events = POLLIN;
            newOutputData->drm.fds.fd = lDevice.fd;
            newOutputData->drm.fds.revents = 0;

            Int32 avaliableNameNumber = 1;

            // Buscamos un número disponible para añadir al final del nombre (E.g HDMI-A-4)

            for(LOutput *o : outputs)
            {
                OUTPUT_DATA *oData = (OUTPUT_DATA*)o->imp()->graphicBackendData;
                if(oData->drm.connector->connector_type == connector->connector_type)
                    avaliableNameNumber++;
            }

            sprintf(newOutputData->drm.name, "%s-%d", (char*)conn_name(connector->connector_type), avaliableNameNumber);

            outputs.push_front(newOutput);

            if(notify)
                lDevice.compositor->outputManager()->outputPlugged(newOutput);

        }
        else
        {
            // Verificamos si la salida estaba conectada
            for(list<LOutput*>::iterator it = outputs.begin(); it != outputs.end(); it++)
            {
                LOutput *output = *it;
                OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
                data->plugged = false;
                if(data->drm.connector->connector_id == connector->connector_id)
                {
                    outputs.erase(it);
                    lDevice.compositor->outputManager()->outputUnplugged(output);
                    lDevice.compositor->removeOutput(output);

                    // Esperamos que el compositor invoque uninitializeOutput() para eliminarla
                    //destroyOutput(output);
                    break;
                }
            }

            drmModeFreeConnector(connector);
            connector = NULL;
        }
    }

    drmModeFreeResources(resources);

}

int hotplugEvent(int, unsigned int, void*)
{
    udev_device* monitorDev = udev_monitor_receive_device(lDevice.monitor);

    if(monitorDev)
    {
        if(udev_device_get_devnum(lDevice.dev) == udev_device_get_devnum(monitorDev))
        {
            ////printf("HOTPLUG EVENT.\n");
            manageOutputs(true);
        }
    }

    udev_device_unref(monitorDev);

    return 0;
}

static void pageFlipHandler(int, unsigned int, unsigned int, unsigned int, void *user_data)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)user_data;
    data->drm.pendingPageFlip = false;
}

static const struct {
    const char *name;
    uint64_t cap;
} client_caps[] = {
    { "STEREO_3D", DRM_CLIENT_CAP_STEREO_3D },
    { "UNIVERSAL_PLANES", DRM_CLIENT_CAP_UNIVERSAL_PLANES },
    { "ATOMIC", DRM_CLIENT_CAP_ATOMIC },
    { "ASPECT_RATIO", DRM_CLIENT_CAP_ASPECT_RATIO },
    { "WRITEBACK_CONNECTORS", DRM_CLIENT_CAP_WRITEBACK_CONNECTORS },
};

static const struct {
    const char *name;
    uint64_t cap;
} caps[] = {
    { "DUMB_BUFFER", DRM_CAP_DUMB_BUFFER },
    { "VBLANK_HIGH_CRTC", DRM_CAP_VBLANK_HIGH_CRTC },
    { "DUMB_PREFERRED_DEPTH", DRM_CAP_DUMB_PREFERRED_DEPTH },
    { "DUMB_PREFER_SHADOW", DRM_CAP_DUMB_PREFER_SHADOW },
    { "PRIME", DRM_CAP_PRIME },
    { "TIMESTAMP_MONOTONIC", DRM_CAP_TIMESTAMP_MONOTONIC },
    { "ASYNC_PAGE_FLIP", DRM_CAP_ASYNC_PAGE_FLIP },
    { "CURSOR_WIDTH", DRM_CAP_CURSOR_WIDTH },
    { "CURSOR_HEIGHT", DRM_CAP_CURSOR_HEIGHT },
    { "ADDFB2_MODIFIERS", DRM_CAP_ADDFB2_MODIFIERS },
    { "PAGE_FLIP_TARGET", DRM_CAP_PAGE_FLIP_TARGET },
    { "CRTC_IN_VBLANK_EVENT", DRM_CAP_CRTC_IN_VBLANK_EVENT },
    { "SYNCOBJ", DRM_CAP_SYNCOBJ },
    { "SYNCOBJ_TIMELINE", DRM_CAP_SYNCOBJ_TIMELINE },
};

void driver_info(int fd)
{
    drmVersion *ver = drmGetVersion(fd);

    if(!ver)
    {
        perror("drmGetVersion");
        return;
    }

    //printf("DRIVER NAME:%s\n", ver->name);
    //printf("DRIVER DESC:%s\n", ver->desc);

    drmFreeVersion(ver);

    for (size_t i = 0; i < sizeof(client_caps) / sizeof(client_caps[0]); ++i)
    {
        bool supported = drmSetClientCap(fd, client_caps[i].cap, 1) == 0;
        //printf("CLIENT_CAP %s = %s\n", client_caps[i].name, supported ? "Yes" : "No");
    }

    for (size_t i = 0; i < sizeof(caps) / sizeof(caps[0]); ++i)
    {
        UInt64 cap;

        if(drmGetCap(fd, caps[i].cap, &cap) == 0)
        {
            //printf("CAP %s = %lu\n",caps[i].name, cap);
        }

    }

}

void getPlanesInfo(int fd)
{
    driver_info(fd);

    drmModePlaneRes *planes = drmModeGetPlaneResources(fd);

    //printf("Planes count: %d\n", planes->count_planes);

    for(UInt32 i = 0; i < planes->count_planes; i++)
    {

        //printf("Plane id: %d\n\n", planes->planes[i]);
        drmModePlane *plane = drmModeGetPlane(fd,planes->planes[i]);

        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);

        for(UInt32 p = 0; p < props->count_props; p++)
        {
            drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[p]);

            if (!prop)
            {
                //printf("DRM error: drmModeGetProperty");
                continue;
            }

            ////printf("Prop name: %s\n", prop->name);
            /*
            if(strcmp("type", prop->name) == 0)
            {
                if(props->prop_values[p] == DRM_PLANE_TYPE_CURSOR)
                    //printf("IS CURSOR\n");
                if(props->prop_values[p] == DRM_PLANE_TYPE_PRIMARY)
                    //printf("IS PRIMARY\n");
                if(props->prop_values[p] == DRM_PLANE_TYPE_OVERLAY)
                    //printf("IS OVERLAY\n");
            }
            */

            if(strcmp("CRTC_X", prop->name) == 0)
            {
                //printf("PROP ID CRTCX %d\n", prop->prop_id);
            }

            drmModeFreeProperty(prop);
        }

        drmModeFreeObjectProperties(props);
        drmModeFreePlane(plane);
    }

    drmModeFreePlaneResources(planes);

}

bool LGraphicBackend::initialize(LCompositor *compositor)
{
    /*
    drm = new LDRM(compositor);

    if(!drm->initialize())
    {
        LLog::error("%s could not be initialized.",LBACKEND_NAME);
        delete drm;
        drm = nullptr;
        return false;
    }

    return true;
    */

    lDevice.compositor = (LCompositor*)compositor;
    lDevice.udev = udev_new();

    if (!lDevice.udev)
    {
        //printf("Can't create udev.\n");
        exit(1);
    }

    udev_enumerate *enumerate;
    udev_list_entry *devices;

    enumerate = udev_enumerate_new(lDevice.udev);
    udev_enumerate_add_match_subsystem(enumerate, "drm");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "drm_minor");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry *entry;

    bool foundDevice = false;
    
    const char *defaultDeviceName = getenv("LOUVRE_DRM_DEVICE");
    if(!defaultDeviceName)
      defaultDeviceName = "/dev/dri/card0";


    // Iteramos las GPUs disponibles
    udev_list_entry_foreach(entry, devices)
    {
        // Path donde la GPU está montada
        const char *path = udev_list_entry_get_name(entry);

        // Obtenemos el handle udev
        lDevice.dev = udev_device_new_from_syspath(lDevice.udev, path);

        // Obtenemos nombre de la GPU (Ej:/dev/dri/card0)
        const char *devName = udev_device_get_property_value(lDevice.dev, "DEVNAME");

        // Verificamos si es igual a la definida en la variable de entorno LOUVRE_DRM_BACKEND_GPU o la por defecto
        if(strcmp(devName, defaultDeviceName) != 0)
          continue;

        LLog::debug("%s Using device %s", LBACKEND_NAME, devName);

        drmModeRes *resources;

        lDevice.fd = open(devName, O_RDWR);

        if(!drmIsMaster(lDevice.fd))
        {
            LLog::fatal("%s Not DRM master. Switch to a free TTY and relaunch.", LBACKEND_NAME);
            close(lDevice.fd);
            exit(1);
        }

        close(lDevice.fd);

        int id;
        id = compositor->seat()->openDevice(devName, &lDevice.fd);

        if (lDevice.fd < 0)
        {
            printf("Could not open drm device\n");
            udev_device_unref(lDevice.dev);
            lDevice.dev = nullptr;
            continue;
        }

        getPlanesInfo(lDevice.fd);

        resources = drmModeGetResources(lDevice.fd);

        if (!resources)
        {
            printf("drmModeGetResources failed: %s\n", strerror(errno));
            drmModeFreeResources(resources);
            udev_device_unref(lDevice.dev);
            lDevice.dev = nullptr;
            compositor->seat()->closeDevice(id);
            continue;
        }

        devicesID[lDevice.fd] = id;


        // Encontramos una GPU disponible !

        drmModeFreeResources(resources);

        foundDevice = true;

        break;
    }

    udev_enumerate_unref(enumerate);

    if(!foundDevice)
    {
        printf("No GPU card found.\n");
        exit(1);
    }

    // Creamos un monitor para escuchar eventos hotplug
    lDevice.monitor = udev_monitor_new_from_netlink(lDevice.udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(lDevice.monitor, "drm", "drm_minor");

    udev_list_entry *tags = udev_device_get_tags_list_entry(lDevice.dev);

    // Añadimos filtros al monitor para escuchar solo los eventos hotplug de la GPU seleccionada
    udev_list_entry_foreach(entry, tags)
    {
        const char *tag = udev_list_entry_get_name(entry);
        udev_monitor_filter_add_match_tag(lDevice.monitor, tag);
    }

    udev_monitor_enable_receiving(lDevice.monitor);

    // Añadimos el descriptor de archivo al loop de eventos de Wayland
    LWayland::addFdListener(udev_monitor_get_fd(lDevice.monitor), nullptr, &hotplugEvent);

    // Genera la lista de todas las salidas conectadas sin notificar al LOutputManager (false)
    manageOutputs(false);

    return true;
}

void LGraphicBackend::uninitialize(const LCompositor *compositor)
{
    L_UNUSED(compositor);

    /* TODO: Permitir cambiar de backend gráfico dinámicamente ? */
}

const list<LOutput*> *LGraphicBackend::getAvaliableOutputs(const LCompositor *compositor)
{
    L_UNUSED(compositor);

    return &outputs;
}

static int match_config_to_visual(EGLDisplay egl_display, EGLint visual_id, EGLConfig *configs, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        EGLint id;

        if (!eglGetConfigAttrib(egl_display, configs[i], EGL_NATIVE_VISUAL_ID, &id))
            continue;

        if (id == visual_id)
            return i;
    }

    return -1;
}

static bool egl_choose_config(EGLDisplay egl_display, const EGLint *attribs, EGLint visual_id, EGLConfig *config_out)
{
    EGLint count = 0;
    EGLint matched = 0;
    EGLConfig *configs;
    int config_index = -1;

    if (!eglGetConfigs(egl_display, NULL, 0, &count) || count < 1)
    {
        LLog::error("No EGL configs to choose from.\n");
        return false;
    }

    configs = (void**)malloc(count * sizeof *configs);

    if (!configs)
        return false;

    if (!eglChooseConfig(egl_display, attribs, configs, count, &matched) || !matched)
    {
        LLog::error("No EGL configs with appropriate attributes.\n");
        goto out;
    }

    if (!visual_id)
    {
        config_index = 0;
    }

    if (config_index == -1)
    {
        config_index = match_config_to_visual(egl_display, visual_id, configs, matched);
    }

    if (config_index != -1)
    {
        *config_out = configs[config_index];
    }

out:
    free(configs);
    if (config_index == -1)
        return false;

    return true;
}

void LGraphicBackend::initializeOutput(const LOutput *output)
{

    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    data->currentBufferIndex = 0;

    data->drm.evctx =
    {
        .version = DRM_EVENT_CONTEXT_VERSION,
        .vblank_handler = NULL,
        .page_flip_handler = pageFlipHandler,
        .page_flip_handler2 = NULL,
        .sequence_handler = NULL
    };

    if(LWayland::isGlContextInitialized())
    {
        LOutput *mainOutput = (LOutput*)LWayland::mainOutput();
        OUTPUT_DATA *mainOutputData = (OUTPUT_DATA*)mainOutput->imp()->graphicBackendData;
        data->gbm.dev = mainOutputData->gbm.dev;
    }
    else
        data->gbm.dev = gbm_create_device(data->drm.fds.fd);

    uint64_t modifier = DRM_FORMAT_MOD_LINEAR;

    EGLint fmt = GBM_FORMAT_XRGB8888;

    data->gbm.surface = nullptr;

    if(false && gbm_surface_create_with_modifiers)
    {
        data->gbm.surface = gbm_surface_create_with_modifiers(data->gbm.dev,
                                data->currentMode->sizeB().w(),data->currentMode->sizeB().h(),
                                fmt,
                                &modifier, 1);

    }

    if(!data->gbm.surface)
        data->gbm.surface = gbm_surface_create(
            data->gbm.dev,
            data->currentMode->sizeB().w(),data->currentMode->sizeB().h(),
            fmt,
            GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

    if (!data->gbm.surface)
    {
        //printf("Failed to create gbm surface.\n");
        return;
    }

    EGLint major, minor;

    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };


    PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display = NULL;
    get_platform_display =(void *(*)(unsigned int,void*,const int*)) eglGetProcAddress("eglGetPlatformDisplayEXT");
    assert(get_platform_display != NULL);

    if(LWayland::isGlContextInitialized())
    {
        data->gl.display = LWayland::eglDisplay();
    }
    else
    {
        if(get_platform_display)
            data->gl.display = get_platform_display(EGL_PLATFORM_GBM_KHR, data->gbm.dev, NULL);
        else
            data->gl.display = eglGetDisplay((EGLNativeDisplayType)data->gbm.dev);

    }

    if (!eglInitialize(data->gl.display, &major, &minor))
    {
        //printf("Failed to initialize EGL.\n");
        return;
    }

    //printf("Using display %p with EGL version %d.%d.\n", data->gl.display, major, minor);
    //printf("EGL Version \"%s\"\n", eglQueryString(data->gl.display, EGL_VERSION));
    //printf("EGL Vendor \"%s\"\n", eglQueryString(data->gl.display, EGL_VENDOR));
    //printf("EGL Extensions \"%s\"\n", eglQueryString(data->gl.display, EGL_EXTENSIONS));

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        //printf("Failed to bind api EGL_OPENGL_ES_API.\n");
        return;
    }



    if (!egl_choose_config(data->gl.display, config_attribs, fmt, &data->gl.config))
    {
        LLog::error("Failed to choose EGL config.\n");
        return;
    }



    /*
    int n;
    if (!eglChooseConfig(data->gl.display, config_attribs, &data->gl.config, 1, &n) || n != 1)
    {
        LLog::error("failed to choose config: %d.\n", n);
    }*/


    EGLContext ctx = EGL_NO_CONTEXT;

    if(LWayland::isGlContextInitialized())
        ctx = LWayland::eglContext();

    if(!data->gl.context)
        data->gl.context = eglCreateContext(data->gl.display, data->gl.config, ctx, context_attribs);

    if (data->gl.context == NULL)
    {
        LLog::error("Failed to create context.\n");
        return;
    }

    PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC create_platform_window = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC) eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");

    data->gl.surface = create_platform_window(data->gl.display, data->gl.config, data->gbm.surface, NULL);
            //eglCreateWindowSurface(data->gl.display, data->gl.config, data->gbm.surface, NULL);

    if (data->gl.surface == EGL_NO_SURFACE)
    {
        LLog::error("Failed to create EGL surface.\n");
        return;
    }

    // connect the context to the surface
    eglMakeCurrent(data->gl.display, data->gl.surface, data->gl.surface, data->gl.context);

    if(!LWayland::isGlContextInitialized())
        LWayland::setContext(output, data->gl.display, data->gl.context);

    //printf("GL Extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));

    // Clear the color buffer
    eglSwapBuffers(data->gl.display, data->gl.surface);
    data->bo = gbm_surface_lock_front_buffer(data->gbm.surface);
    gbm_bo_set_user_data(data->bo, NULL, NULL);
    data->fb = drm_fb_get_from_bo(data->bo,data)->fb;

    // set mode:
    OUTPUT_MODE *om = (OUTPUT_MODE*)data->currentMode->imp()->graphicBackendData;

    ret = drmModeSetCrtc(data->drm.fds.fd, data->drm.crtc_id, data->fb->fb_id, 0, 0,
                         &data->drm.connector->connector_id,
                         1,
                         om->mode);

    if (ret)
    {
        printf("Failed to set mode: %s\n", strerror(errno));
        return;
    }

    output->imp()->state = LOutput::Initialized;
    data->initialized = true;

    return;
}

void LGraphicBackend::uninitializeOutput(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    // Si no estaba inicializada no hacemos nada
    if(data->initialized)
    {
        // Destruimos la superficie EGL
        eglDestroySurface(data->gl.display, data->gl.surface);

        // Destruimos el contexto EGL
        //eglDestroyContext(data->gl.display, data->gl.context);

        // Destruimos la superficie GBM
        gbm_surface_destroy(data->gbm.surface);

        // Destruimos el dispositivo GBM
        //gbm_device_destroy(data->gbm.dev);
    }

    // Elimina cursor
    if(data->cursor.initialized)
    {
        setCursorTexture(output, nullptr, LSizeF());
        data->cursor.initialized = false;
        glDeleteFramebuffers(1,&data->cursor.fb);
        glDeleteTextures(1,&data->cursor.texture);
        eglDestroyImage(data->gl.display,data->cursor.eglImage);
        gbm_bo_destroy(data->cursor.bo);
    }

    // Si no está conectada la eliminamos
    if(!data->plugged)
    {
        destroyOutput((LOutput*)output);
    }

}

void LGraphicBackend::flipOutputPage(const LOutput *output)
{

    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    gbm_bo *next_bo;

    if(output->compositor()->seat()->enabled())
    {
        if(output->state() == LOutput::Suspended)
        {
            drmSetMaster(data->drm.fds.fd);
            OUTPUT_MODE *mod = (OUTPUT_MODE*)data->currentMode->imp()->graphicBackendData;
            drmModeSetCrtc(data->drm.fds.fd, data->drm.crtc_id, data->fb->fb_id, 0, 0, &data->drm.connector->connector_id, 1, mod->mode);
            output->imp()->state = LOutput::Initialized;
        }
    }
    else
    {
        output->compositor()->imp()->renderMutex.unlock();
        return;
    }

    eglSwapBuffers(data->gl.display, data->gl.surface);
    output->compositor()->imp()->renderMutex.unlock();

    next_bo = gbm_surface_lock_front_buffer(data->gbm.surface);
    data->fb = drm_fb_get_from_bo(next_bo,data)->fb;

    data->drm.pendingPageFlip = true;
    drmModePageFlip(data->drm.fds.fd, data->drm.crtc_id, data->fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, data);

    // Previene que otras salidas invoken el método drmHandleEvent al mismo tiempo lo que causa bugs
    pageFlipMutex.lock();

    while(data->drm.pendingPageFlip)
    {
        poll(&data->drm.fds, 1, 100);

        if(data->drm.fds.revents & POLLIN)
            drmHandleEvent(data->drm.fds.fd, &data->drm.evctx);

        if(!output->compositor()->seat()->enabled() || output->state() != LOutput::Initialized)
            break;
    }

    pageFlipMutex.unlock();

    // release last buffer to render on again:
    gbm_surface_release_buffer(data->gbm.surface, data->bo);
    data->bo = next_bo;
    data->currentBufferIndex = 1 - data->currentBufferIndex;
}

EGLDisplay LGraphicBackend::getOutputEGLDisplay(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->gl.display;
}

const LSize *LGraphicBackend::getOutputPhysicalSize(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return &data->physicalSize;
}

Int32 LGraphicBackend::getOutputCurrentBufferIndex(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->currentBufferIndex;
}

const char *LGraphicBackend::getOutputName(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->drm.name;
}

const char *LGraphicBackend::getOutputManufacturerName(const LOutput *output)
{
    L_UNUSED(output);
    return "Unknown Manufacturer";
}

const char *LGraphicBackend::getOutputModelName(const LOutput *output)
{
    L_UNUSED(output);
    return "Unknown Model";
}

const char *LGraphicBackend::getOutputDescription(const LOutput *output)
{
    L_UNUSED(output);
    return "A DRM output.";
}

const LOutputMode *LGraphicBackend::getOutputPreferredMode(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->preferredMode;
}

const LOutputMode *LGraphicBackend::getOutputCurrentMode(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->currentMode;
}

const std::list<LOutputMode *> *LGraphicBackend::getOutputModes(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return &data->modes;
}

void LGraphicBackend::setOutputMode(const LOutput *output, const LOutputMode *mode)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    data->currentMode = (LOutputMode*)mode;

    if(data->initialized)
    {
        // Destruimos la superficie EGL
        eglDestroySurface(data->gl.display, data->gl.surface);

        // Destruimos la superficie GBM
        gbm_surface_destroy(data->gbm.surface);

        initializeOutput(output);
    }
}

const LSize *LGraphicBackend::getOutputModeSize(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return &data->size;
}

Int32 LGraphicBackend::getOutputModeRefreshRate(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return data->mode->vrefresh*1000;
}

bool LGraphicBackend::getOutputModeIsPreferred(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return data->isPreferred;
}

int dumbCursor(OUTPUT_DATA *data)
{
    struct drm_mode_create_dumb create_request = {
        .height = 64,
        .width  = 64,
        .bpp    = 32
    };

    ret = ioctl(data->drm.fds.fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_request);

    if(ret)
        return 0;

    ret = drmModeAddFB(
        data->drm.fds.fd,
        64, 64,
        24, 32, create_request.pitch,
        create_request.handle, &data->cursor.fbId
    );

    if(ret)
        return 0;

    struct drm_prime_handle prime_request = {
        .handle = create_request.handle,
        .flags  = DRM_CLOEXEC | DRM_RDWR,
        .fd     = -1
    };

    ret = ioctl(data->drm.fds.fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime_request);
    data->cursor.fd = prime_request.fd;
    data->cursor.handle = prime_request.handle;

    if (ret || data->cursor.fd < 0)
       return 0;

    data->cursor.buffer = (UChar8*)mmap(
        0, create_request.size,	PROT_READ | PROT_WRITE, MAP_SHARED,
        data->cursor.fd, 0);

    ret = errno;

    if(data->cursor.buffer == NULL || data->cursor.buffer == MAP_FAILED)
        return 0;

    return 1;
}

void LGraphicBackend::initializeCursor(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    if(data->cursor.initialized)
        return;

    data->cursor.initialized = true;

    data->cursor.isDumb = dumbCursor(data);

    // Create cursor bo
    data->cursor.bo = gbm_bo_create(data->gbm.dev, 64, 64, GBM_FORMAT_ARGB8888, GBM_BO_USE_CURSOR | GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    // Cursor
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress ("eglCreateImageKHR");
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress ("glEGLImageTargetTexture2DOES");
    data->cursor.eglImage = eglCreateImageKHR(data->gl.display, data->gl.context, EGL_NATIVE_PIXMAP_KHR, data->cursor.bo, NULL);


    glGenFramebuffers(1, &data->cursor.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, data->cursor.fb);

    glActiveTexture(GL_TEXTURE0+1);
    glGenTextures(1, &data->cursor.texture);
    glBindTexture(GL_TEXTURE_2D, data->cursor.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,data->cursor.eglImage);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->cursor.texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool LGraphicBackend::hasHardwareCursorSupport()
{
    return true;
}

void LGraphicBackend::setCursorTexture(const LOutput *output, const LTexture *texture, const LSizeF &size)
{
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

}

void LGraphicBackend::setCursorPosition(const LOutput *output, const LPoint &position)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    drmModeMoveCursor(data->drm.fds.fd,
                      data->drm.crtc_id,
                      position.x(),
                      position.y());
}

LGraphicBackendInterface API;

extern "C" LGraphicBackendInterface *getAPI()
{
    API.initialize = &LGraphicBackend::initialize;
    API.uninitialize = &LGraphicBackend::uninitialize;
    API.getAvaliableOutputs = &LGraphicBackend::getAvaliableOutputs;
    API.initializeOutput = &LGraphicBackend::initializeOutput;
    API.uninitializeOutput = &LGraphicBackend::uninitializeOutput;
    API.flipOutputPage = &LGraphicBackend::flipOutputPage;
    API.getOutputEGLDisplay = &LGraphicBackend::getOutputEGLDisplay;
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
    API.initializeCursor = &LGraphicBackend::initializeCursor;
    API.hasHardwareCursorSupport = &LGraphicBackend::hasHardwareCursorSupport;
    API.setCursorTexture = &LGraphicBackend::setCursorTexture;
    API.setCursorPosition = &LGraphicBackend::setCursorPosition;
    return &API;
}

