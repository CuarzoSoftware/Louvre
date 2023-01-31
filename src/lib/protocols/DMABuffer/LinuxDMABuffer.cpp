#include "LinuxDMABuffer.h"
#include "linux-dmabuf-unstable-v1.h"
#include "Params.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <LWayland.h>
#include <LLog.h>
#include <cstring>
#include <fcntl.h>
#include <private/LCompositorPrivate.h>


#include <cassert>
#include <libdrm/drm_fourcc.h>

#include <LCompositor.h>
#include <private/LClientPrivate.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "DMA.h"

using namespace Louvre::Extensions::LinuxDMABuffer;

static const struct
{
    const char *name;
    uint64_t cap;
}
client_caps[] =
{
    { "STEREO_3D", DRM_CLIENT_CAP_STEREO_3D },
    { "UNIVERSAL_PLANES", DRM_CLIENT_CAP_UNIVERSAL_PLANES },
    { "ATOMIC", DRM_CLIENT_CAP_ATOMIC },
    { "ASPECT_RATIO", DRM_CLIENT_CAP_ASPECT_RATIO },
    { "WRITEBACK_CONNECTORS", DRM_CLIENT_CAP_WRITEBACK_CONNECTORS },
};

static const struct
{
    const char *name;
    uint64_t cap;
}
caps[] =
{
    { "DUMB_BUFFER", DRM_CAP_DUMB_BUFFER },
    { "VBLANK_HIGH_CRTC", DRM_CAP_VBLANK_HIGH_CRTC },
    { "DUMB_PREFERRED_DEPTH", DRM_CAP_DUMB_PREFERRED_DEPTH },
    { "DUMB_PREFER_SHADOW", DRM_CAP_DUMB_PREFER_SHADOW },
    { "PRIME", DRM_CAP_PRIME },
    { "PRIME_IMPORT", DRM_PRIME_CAP_IMPORT },
    { "PRIME_EXPORT", DRM_PRIME_CAP_EXPORT },
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

int fd;
const char *name;
bool capDumbBuffer;
bool capAddFB2Mod;
bool capPrimeImport;
bool capPrimeExport;
drmModeRes *res;

bool updateDriverInfo()
{
    drmVersion *version = drmGetVersion(fd);

    /*
    LLog::debug("Getting driver info for device %s:", name);

    if(!version)
    {
        LLog::error("Could not get driver version.");
        drmFreeVersion(version);
        return false;
    }

    LLog::debug("Driver: %s (%s) version %d.%d.%d (%s)",
           version->name,
           version->desc,
           version->version_major,
           version->version_minor,
           version->version_patchlevel,
           version->date);
    */

    drmFreeVersion(version);

    /*
    LLog::debug("Client capabilities:");

    for(size_t i = 0; i < sizeof(client_caps) / sizeof(client_caps[0]); i++)
    {
        bool supported = drmSetClientCap(fd, client_caps[i].cap, 1) == 0;
        LLog::debug("\tCLIENT_CAP_%s = %s", client_caps[i].name, supported ? "Yes" : "No");
    }

    LLog::debug("Capabilities:");*/

    for(size_t i = 0; i < sizeof(caps) / sizeof(caps[0]); i++)
    {
        UInt64 cap;

        if(drmGetCap(fd, caps[i].cap, &cap) == 0)
        {
            // LLog::debug("\tCAP_%s = %lu", caps[i].name, cap);

            if(caps[i].cap == DRM_CAP_DUMB_BUFFER)
               capDumbBuffer = cap == 1;
            else if(caps[i].cap == DRM_CAP_ADDFB2_MODIFIERS)
               capAddFB2Mod = cap == 1;
            else if(caps[i].cap == DRM_PRIME_CAP_IMPORT)
               capPrimeImport = cap == 1;
            else if(caps[i].cap == DRM_PRIME_CAP_EXPORT)
               capPrimeExport = cap == 1;
        }

    }

    return true;
}

void send_format(struct wl_resource *resource, uint32_t format, uint64_t modifier)
{
    uint32_t hi = modifier >> 32;
    uint32_t lo = modifier & 0xFFFFFFFF;
    zwp_linux_dmabuf_v1_send_modifier(resource, format, hi, lo);
}

static char printable_char(int c)
{
    return isascii(c) && isprint(c) ? c : '?';
}

const char *drm_get_format_name(uint32_t format)
{
   static char buf[32];

   snprintf(buf, sizeof(buf),
        "%c%c%c%c %s-endian (0x%08x)",
        printable_char(format & 0xff),
        printable_char((format >> 8) & 0xff),
        printable_char((format >> 16) & 0xff),
        printable_char((format >> 24) & 0x7f),
        format & DRM_FORMAT_BIG_ENDIAN ? "big" : "little",
        format);

   return buf;
}
void get_drm_formats( wl_resource *linuxDMABufResource )
{

    EGLDeviceEXT egl_dev;

    PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttrib = (PFNEGLQUERYDISPLAYATTRIBEXTPROC) eglGetProcAddress ("eglQueryDisplayAttribEXT");

    queryDisplayAttrib(LWayland::eglDisplay(), EGL_DEVICE_EXT, (EGLAttribKHR*)&egl_dev);

    PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT");

    name = eglQueryDeviceStringEXT(egl_dev,EGL_DRM_DEVICE_FILE_EXT);

    printf("DRM DEVICE %s\n", name);

    fd = open(name, O_RDONLY);

    printf("FD %d\n", fd);

    updateDriverInfo();

    res = drmModeGetResources(fd);

    drmModePlaneRes *planeResources = drmModeGetPlaneResources(fd);

    if(!planeResources)
    {
        LLog::error("Could not get device planes.");
        return;
    }

    for(UInt32 i = 0; i < planeResources->count_planes; i++)
    {

        printf("Plane--------------- %d\n", i);
        drmModePlane *planeResource = drmModeGetPlane(fd, planeResources->planes[i]);

        if(!planeResource)
        {
            LLog::error("Could not get info of plane (%d):", planeResources->planes[i]);
            continue;
        }
        int id = planeResource->plane_id;
        drmModeObjectProperties *planeProperties = drmModeObjectGetProperties(fd, id , DRM_MODE_OBJECT_PLANE);

        if(!planeProperties)
        {
            LLog::error("Failed to get properties of plane (%d).", id);
            return;
        }

        UInt64 type;

        for(UInt32 i = 0; i < planeProperties->count_props; i++)
        {
            drmModePropertyRes *prop = drmModeGetProperty(fd, planeProperties->props[i]);


            if (!prop)
            {
                LLog::error("Failed to get property (%d) of plane (%d).", planeProperties->props[i], id);
                continue;
            }

            if(strcmp("type", prop->name) == 0)
            {
                type = planeProperties->prop_values[i];
                if(type == DRM_PLANE_TYPE_CURSOR)
                {
                    LLog::debug("\t Type: %s\n", "CURSOR");
                }
                else if(type == DRM_PLANE_TYPE_PRIMARY)
                {
                    LLog::debug("\t Type: %s\n", "PRIMARY");
                }
                else if(type == DRM_PLANE_TYPE_OVERLAY)
                {
                    LLog::debug("\t Type: %s\n", "OVERLAY");
                }
                else
                {
                    type = -1;
                    LLog::debug("\t Type: %s\n", "UNKNOW");
                }
            }
            else if(strcmp("IN_FORMATS", prop->name) == 0)
            {
                drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(fd, planeProperties->prop_values[i]);

                if(!blob)
                {
                    LLog::error("\t Could not get IN_FORMATS blob.");
                    goto freeProp;
                }

                drmModeFormatModifierIterator iter;

                while (drmModeFormatModifierBlobIterNext(blob, &iter))
                {

                    printf("Format: %s, Mod: %s\n", drm_get_format_name(iter.fmt), drmGetFormatModifierName(iter.mod));
                    send_format(linuxDMABufResource, iter.fmt, iter.mod);
                    send_format(linuxDMABufResource, iter.fmt, DRM_FORMAT_MOD_INVALID);
                    /*inFormat.format = iter.fmt;
                    inFormat.mod = iter.mod;
                    inFormats().push_back(inFormat);*/
                }

                drmModeFreePropertyBlob(blob);

            }



            freeProp:
            drmModeFreeProperty(prop);
        }

        drmModeFreeObjectProperties(planeProperties);

    }

}


static struct zwp_linux_dmabuf_v1_interface zwp_linux_dmabuf_v1_implementation =
{
    .destroy = &LinuxDMABuffer::destroy,
    .create_params = &LinuxDMABuffer::create_params,
    .get_default_feedback = &LinuxDMABuffer::get_default_feedback,
    .get_surface_feedback = &LinuxDMABuffer::get_surface_feedback
};

static struct zwp_linux_buffer_params_v1_interface zwp_linux_buffer_params_v1_implementation
{
    .destroy = &Params::destroy,
    .add = &Params::add,
    .create = &Params::create,
    .create_immed = &Params::create_immed
};

void LinuxDMABuffer::resource_destroy(wl_resource *resource)
{

}

void LinuxDMABuffer::destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void LinuxDMABuffer::create_params(wl_client *client, wl_resource *resource, UInt32 id)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    wl_resource *params = wl_resource_create(client,
                                             &zwp_linux_buffer_params_v1_interface,
                                             wl_resource_get_version(resource),
                                             id);

    LLog::log("Create params DMA");

    LDMAParams *lParams = new LDMAParams();
    lParams->client = lClient;
    lParams->resource = params;

    wl_resource_set_implementation(params,
                                   &zwp_linux_buffer_params_v1_implementation,
                                   lParams,
                                   &Params::resource_destroy);
}

void LinuxDMABuffer::get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id)
{
    LLog::log("get default feedback DMA");
}

void LinuxDMABuffer::get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    LLog::log("get surface feedback DMA");
}

void LinuxDMABuffer::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = lCompositor->getClientFromNativeResource(client);

    if(!lClient)
        return;

    /*
    if(lClient->imp()->linuxDMABufResource)
    {
        LLog::log("Already DMA");
        return;
    }*/

    lClient->imp()->linuxDMABufResource = wl_resource_create(client, &zwp_linux_dmabuf_v1_interface, version, id);

    wl_resource_set_implementation(lClient->imp()->linuxDMABufResource, &zwp_linux_dmabuf_v1_implementation, lClient, &LinuxDMABuffer::resource_destroy);


    if (wl_resource_get_version(lClient->imp()->linuxDMABufResource) >= 3)
    {
        LLog::log("DMA version %d", version);
        get_drm_formats(lClient->imp()->linuxDMABufResource);
        /*
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_C8, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XBGR8888, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB2101010, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XBGR2101010, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB16161616F, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XBGR16161616F, DRM_FORMAT_MOD_INVALID);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_RGB565, DRM_FORMAT_MOD_INVALID);
        */



        /*
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR);

        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888, I915_FORMAT_MOD_X_TILED);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888, I915_FORMAT_MOD_X_TILED);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_BGRX8888, DRM_FORMAT_MOD_LINEAR);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_BGRX8888, I915_FORMAT_MOD_X_TILED);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_BGRA8888, DRM_FORMAT_MOD_LINEAR);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_BGRA8888, I915_FORMAT_MOD_X_TILED);


        LLog::log("DRM %d", DRM_FORMAT_BGRA8888);
        LLog::log("DRM %d", DRM_FORMAT_ABGR8888);
        LLog::log("DRM %d", DRM_FORMAT_ARGB8888);
        LLog::log("DRM %d", DRM_FORMAT_RGBA8888);

        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV, DRM_FORMAT_MOD_LINEAR);
        send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV, I915_FORMAT_MOD_X_TILED);
        */
    } else
    {
        zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888);
        zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888);
        //zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV);
    }
}
