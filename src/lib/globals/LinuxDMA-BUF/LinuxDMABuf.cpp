#ifdef LOUVRE_DMA_ENABLE
#include "LinuxDMABuf.h"
#include "wl_buffer_dmabuf.h"
#include "linux-dmabuf-unstable-v1.h"
#include "LGbm.h"
#include "Params.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <LWayland.h>
#include <private/LCompositorPrivate.h>


#include <cassert>
#include <libdrm/drm_fourcc.h>

#include <LCompositor.h>
#include <private/LClientPrivate.h>
#include <xf86drmMode.h>

using namespace Louvre;

struct linux_dmabuf
{
    struct buffer_dmabuf_events buffer_dmabuf_events;
};



struct props {
    struct {
        uint32_t type;
        uint32_t src_x;
        uint32_t src_y;
        uint32_t src_w;
        uint32_t src_h;
        uint32_t crtc_x;
        uint32_t crtc_y;
        uint32_t crtc_w;
        uint32_t crtc_h;
        uint32_t fb_id;
        uint32_t in_fence_fd;
        uint32_t crtc_id;
    } plane;

    struct {
        uint32_t crtc_id;
    } conn;

    struct {
        uint32_t out_fence_ptr;
    } crtc;
};

static uint32_t crtc;
static uint32_t conn;
static uint32_t plane;
static struct props props;
static struct wl_resource *to_release[2];

static struct zwp_linux_dmabuf_v1_interface zwp_linux_dmabuf_v1_implementation =
{
    .destroy = &Extensions::LinuxDMABuf::LinuxDMABuf::destroy,
    .create_params = &Extensions::LinuxDMABuf::LinuxDMABuf::create_params,
    .get_default_feedback = &Extensions::LinuxDMABuf::LinuxDMABuf::get_default_feedback,
    .get_surface_feedback = &Extensions::LinuxDMABuf::LinuxDMABuf::get_surface_feedback
};

static struct zwp_linux_buffer_params_v1_interface zwp_linux_buffer_params_v1_implementation
{
    .destroy = &Extensions::LinuxDMABuf::Params::destroy,
    .add = &Extensions::LinuxDMABuf::Params::add,
    .create = &Extensions::LinuxDMABuf::Params::create,
    .create_immed = &Extensions::LinuxDMABuf::Params::create_immed
};

int modeset_get_fd() { return LWayland::drmFd(); }

void atomic_commit(uint32_t buf_id)
{
    int r, fd = modeset_get_fd();
//	if (width != 24) {
        drmModeAtomicReqPtr req = drmModeAtomicAlloc();
        if (req == NULL)
        {
            printf("drmModeAtomicAlloc");
            exit(0);
        }
        drmModeAtomicAddProperty(req, plane, props.plane.fb_id, buf_id);
        r = drmModeAtomicCommit(fd, req, DRM_MODE_PAGE_FLIP_EVENT | DRM_MODE_ATOMIC_NONBLOCK, NULL);
        if (r != 0)
        {
            printf("drmModeAtomicCommit");
            exit(0);
        }
        drmModeAtomicFree(req);
//	}

    /*uint64_t old_buf_id = find_old(fd);
    sleep(1);

    req = drmModeAtomicAlloc();
    if (req == NULL) die("drmModeAtomicAlloc");
    drmModeAtomicAddProperty(req, 40, 16, old_buf_id);
    r = drmModeAtomicCommit(fd, req, 0, NULL);
    if (r != 0) die("drmModeAtomicCommit");
    drmModeAtomicFree(req);*/
}

static void dmabuf(struct wl_resource *dmabuf)
{
    struct bufstate *bufstate = (struct bufstate *)wl_buffer_dmabuf_get_subsystem_object(dmabuf,SUBSYSTEM_DRM);

    atomic_commit(bufstate->fb_id);

    if (!to_release[0])
        to_release[0] = dmabuf;
    else if (!to_release[1])
        to_release[1] = dmabuf;
    else
        assert(false);
}

uint32_t modeset_add_fb(uint32_t width, uint32_t height, uint32_t format, uint32_t handle, uint32_t pitch, uint32_t offset, uint64_t mod)
{
    int r, fd = modeset_get_fd();
    uint32_t bo_handles[4] = {handle}, pitches[4] = {pitch}, offsets[4] = {offset};
    uint64_t modifier[4] = {mod};
    uint32_t buf_id, flags = DRM_MODE_FB_MODIFIERS;

    // remove alpha otherwise fails on my (intel) gpu
    format = format == DRM_FORMAT_ARGB8888 ? DRM_FORMAT_XRGB8888 : format;

    r = drmModeAddFB2WithModifiers(fd, width, height, format, bo_handles,
                               pitches, offsets, modifier, &buf_id, flags);
    if (r != 0)
    {
        printf("drmModeAddFB2WithModifiers\n");
        exit(0);
    }

    printf("success: %dx%d\n", width, height);
    return buf_id;
}

void buffer_dmabuf_create_notify(struct wl_buffer_dmabuf_data *dmabuf, void *user_data)
{
    uint32_t num_planes = 1;
    int32_t *fds = dmabuf->fds;
    uint32_t width = dmabuf->width;
    uint32_t height = dmabuf->height;
    uint32_t format = dmabuf->format;
    uint32_t *strides = dmabuf->strides;
    uint32_t *offsets = dmabuf->offsets;
    uint64_t *mods = dmabuf->modifiers;



    static const int general_attribs = 3;
    static const int plane_attribs = 5;
    static const int entries_per_attrib = 2;
    EGLint attribs[(general_attribs + (plane_attribs)) * entries_per_attrib + 1];
    unsigned int atti = 0;

    attribs[atti++] = EGL_WIDTH;
    attribs[atti++] = width;
    attribs[atti++] = EGL_HEIGHT;
    attribs[atti++] = height;
    attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
    attribs[atti++] = format;

    attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attribs[atti++] = fds[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attribs[atti++] = offsets[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attribs[atti++] = strides[0];
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    attribs[atti++] = mods[0] & 0xFFFFFFFF;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    attribs[atti++] = mods[0] >> 32;



    struct gbm_bo *bo = gbm_import_from_dmabuf(num_planes, fds, width, height, format, strides, offsets, mods[0]);
    // TODO: migrate to drmPrimeFDToHandle (but refcounting issues?)
    assert(bo);

    printf("BO CREATED\n");

    struct bufstate *bufstate = ( struct bufstate *)malloc(sizeof(*bufstate));
    bufstate->bo = bo;
    bufstate->fb_id = modeset_add_fb(width, height, format, gbm_get_handle(bo), strides[0], offsets[0], mods[0]);

    // Cursor
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress ("eglCreateImageKHR");
    bufstate->eglImage = eglCreateImageKHR(LWayland::eglDisplay(), LWayland::eglContext(), EGL_NATIVE_PIXMAP_KHR, bufstate->bo, NULL);

    /*bufstate->eglImage = eglCreateImageKHR(LWayland::eglContext(),
                                  EGL_NO_CONTEXT,
                                  EGL_LINUX_DMA_BUF_EXT,
                                  NULL, attribs);*/

    dmabuf->subsystem_object[SUBSYSTEM_DRM] = bufstate;


}

void modeset_rem_fb(uint32_t buf_id) {
    int r, fd = modeset_get_fd();

    r = drmModeRmFB(fd, buf_id);
    if (r != 0) printf("drmModeAddFB2WithModifiers");
}

void buffer_dmabuf_destroy_notify(struct wl_buffer_dmabuf_data *dmabuf, void*user_data)
{
    struct bufstate *bufstate = (struct bufstate *)dmabuf->subsystem_object[SUBSYSTEM_DRM];
    // TODO: Not sure if we can destroy it immediately
    modeset_rem_fb(bufstate->fb_id);
    gbm_destroy(bufstate->bo);
    free(bufstate);
}

void Extensions::LinuxDMABuf::LinuxDMABuf::resource_destroy(wl_resource *resource)
{

}

void Extensions::LinuxDMABuf::LinuxDMABuf::destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}



void Extensions::LinuxDMABuf::LinuxDMABuf::create_params(wl_client *client, wl_resource *resource, UInt32 id)
{
    printf("DMA Create params\n");
    struct linux_dmabuf *linux_dmabuf = (struct linux_dmabuf *)wl_resource_get_user_data(resource);
    int version = wl_resource_get_version(resource);
    struct wl_resource *child = wl_resource_create(client, &zwp_linux_buffer_params_v1_interface, version, id);

    struct zwp_linux_buffer_params_v1_data* data = (struct zwp_linux_buffer_params_v1_data*)calloc(1, sizeof(*data));
    wl_list_init(&data->plane_list);
    data->buffer_dmabuf_events = linux_dmabuf->buffer_dmabuf_events;
    wl_resource_set_implementation(child, &zwp_linux_buffer_params_v1_implementation, data, &Extensions::LinuxDMABuf::Params::resource_destroy);
}

void Extensions::LinuxDMABuf::LinuxDMABuf::get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id)
{

}

void Extensions::LinuxDMABuf::LinuxDMABuf::get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{

}

void send_format(struct wl_resource *resource, uint32_t format, uint64_t modifier) {
    uint32_t hi = modifier >> 32;
    uint32_t lo = modifier & 0xFFFFFFFF;
    zwp_linux_dmabuf_v1_send_modifier(resource, format, hi, lo);
}

void Extensions::LinuxDMABuf::LinuxDMABuf::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    printf("BIND DMA\n");
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = nullptr;

    for(LClient *c : lCompositor->clients())
    {
        if(c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    if(!lClient)
        return;

    /*
    if(lClient->imp()->linuxDMABufResource)
    {
        printf("Client already DMA\n");
        //wl_client_destroy(client);
        return;
    }
    */
    lClient->imp()->linuxDMABufResource = wl_resource_create(client, &zwp_linux_dmabuf_v1_interface, version, id);

    struct buffer_dmabuf_events buffer_dmabuf_events =
    {
        .create = buffer_dmabuf_create_notify,
        .destroy = buffer_dmabuf_destroy_notify,
        .user_data = lClient
    };

    struct linux_dmabuf *linux_dmabuf = (struct linux_dmabuf *)malloc(sizeof(struct linux_dmabuf));
    linux_dmabuf->buffer_dmabuf_events = buffer_dmabuf_events;
    wl_resource_set_implementation(lClient->imp()->linuxDMABufResource, &zwp_linux_dmabuf_v1_implementation, linux_dmabuf, &Extensions::LinuxDMABuf::LinuxDMABuf::resource_destroy);

    /*
     * Some common formats (Intel GPU)
     */
        if (wl_resource_get_version(lClient->imp()->linuxDMABufResource) >= 3)
        {
            send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR);
            send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888, I915_FORMAT_MOD_X_TILED);
            send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR);
            send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888, I915_FORMAT_MOD_X_TILED);
            //send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV, DRM_FORMAT_MOD_LINEAR);
            //send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV, I915_FORMAT_MOD_X_TILED);
        } else
        {
            zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_XRGB8888);
            zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_ARGB8888);
            zwp_linux_dmabuf_v1_send_format(lClient->imp()->linuxDMABufResource, DRM_FORMAT_YUYV);
        }
}

#endif
