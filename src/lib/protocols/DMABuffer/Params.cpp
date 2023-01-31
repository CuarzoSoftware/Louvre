#include "Params.h"
#include "LLog.h"
#include <protocols/DMABuffer/DMA.h>
#include <protocols/DMABuffer/linux-dmabuf-unstable-v1.h>
#include <EGL/eglext.h>
#include <EGL/egl.h>
#include <cstdio>
#include <libdrm/drm_fourcc.h>
#include <unistd.h>
#include <LWayland.h>


using namespace Louvre;

static void wl_buffer_resource_destroy(struct wl_resource *resource)
{

}

static void wl_buffer_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface wl_buffer_implementation = {wl_buffer_destroy};


bool wl_buffer_is_dmabuf(wl_resource *resource)
{
    return false;
    return wl_resource_instance_of(resource, &wl_buffer_interface, &wl_buffer_implementation);
}

void Extensions::LinuxDMABuffer::Params::resource_destroy(wl_resource *resource)
{
    LDMAParams *lParams = (LDMAParams*)wl_resource_get_user_data(resource);
    delete lParams;
}

void Extensions::LinuxDMABuffer::Params::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void Extensions::LinuxDMABuffer::Params::add(wl_client *client, wl_resource *resource, int fd, UInt32 plane_idx, UInt32 offset, UInt32 stride, UInt32 modifier_hi, UInt32 modifier_lo)
{
    LDMAParams *lParams = (LDMAParams*)wl_resource_get_user_data(resource);

    for(LDMAPlane &plane : lParams->planes)
    {
        if(plane.index == plane_idx)
        {
            wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET, "Plane index was already set.");
            return;
        }
    }

    LDMAPlane plane;
    plane.fd = fd;
    plane.index = plane_idx;
    plane.offset = offset;
    plane.stride = stride;
    plane.modifier = ((uint64_t)modifier_hi << 32) | modifier_lo;
    lParams->planes.push_back(plane);
}

size_t fd_get_size(int fd) {
    size_t curr = lseek(fd, 0, SEEK_CUR);
    size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, curr, SEEK_SET);
    return size;
}

uint32_t format_get_plane_number(uint32_t format) {
    switch (format) {
        default:
        return 1;
    }
}

void create_(struct wl_client *client, struct wl_resource *resource, uint32_t buffer_id, int32_t width, int32_t height, uint32_t format, uint32_t flags)
{
    LDMAParams *lParams = (LDMAParams*)wl_resource_get_user_data(resource);
    wl_resource *buffer = wl_resource_create(client, &wl_buffer_interface, 1, buffer_id);
    LDMABuffer *lDMABuffer = new LDMABuffer();

    lDMABuffer->flags = flags;
    lDMABuffer->format = format;
    lDMABuffer->width = width;
    lDMABuffer->height = height;
    lDMABuffer->resource = buffer;

    LLog::log("Planes %d", (int)lParams->planes.size());

    LDMAPlane &plane = lParams->planes.front();
    EGLAttrib attribs[6 + 10 + 10];
    UInt32 atti = 0;

    attribs[atti++] = EGL_WIDTH;
    attribs[atti++] = width;
    attribs[atti++] = EGL_HEIGHT;
    attribs[atti++] = height;
    attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
    attribs[atti++] = format;

    attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attribs[atti++] = plane.fd;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attribs[atti++] = plane.offset;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attribs[atti++] = plane.stride;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    attribs[atti++] = plane.modifier & 0xFFFFFFFF;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    attribs[atti++] = plane.modifier >> 32;
    attribs[atti++] = EGL_NONE;

    lDMABuffer->eglImage = eglCreateImage(LWayland::eglDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

    if(lDMABuffer->eglImage == EGL_NO_IMAGE)
    {
        LLog::log("FAILED EGL IMAGE");
        exit(1);
    }

    wl_resource_set_implementation(buffer, &wl_buffer_implementation, lDMABuffer, &wl_buffer_resource_destroy);

    if(buffer_id == 0)
        zwp_linux_buffer_params_v1_send_created(resource, buffer);

    LLog::log("DMA BUFFER CREATED\n");


    /*
    struct wl_resource *child;
    struct wl_buffer_dmabuf_data *dmabuf;
    uint32_t num_planes;
    uint32_t num_added;

    if (params->already_used)
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED, "The dmabuf_batch object has already been used to create a wl_buffer");
        goto failed;
    }
    params->already_used = true;

    if (format != DRM_FORMAT_XRGB8888 && format != DRM_FORMAT_ARGB8888 && format != DRM_FORMAT_YUYV) {
        wl_resource_post_error(resource,
         ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT,
          "Format not supported");
        goto failed;
    }

    if (width <= 0) {
        wl_resource_post_error(resource,
         ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
          "Invalid width");
        goto failed;
    }
    if (height <= 0) {
        wl_resource_post_error(resource,
         ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
          "Invalid height");
        goto failed;
    }

    num_planes = format_get_plane_number(format);
    num_added = wl_list_length(&params->plane_list);
    if (num_added < num_planes) {
        wl_resource_post_error(resource,
         ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
          "Missing planes to create a buffer");
        goto failed;
    }
    if (num_added > num_planes) {
        wl_resource_post_error(resource,
         ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
          "Too many planes to create a buffer");
        goto failed;
    }

    LLog::log("BUFF A");
    child = wl_resource_create(client, &wl_buffer_interface, 1, buffer_id);
    dmabuf = wl_buffer_dmabuf_new(child, width, height, format, flags, num_planes, params->buffer_dmabuf_events);
    LLog::log("BUFF B");

    struct plane *plane;
    wl_list_for_each(plane, &params->plane_list, link)
    {
        uint32_t i = plane->plane_idx;
        if (i >= dmabuf->num_planes) {
            wl_resource_post_error(resource,
             ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
              "Plane index out of bounds");
            wl_resource_destroy(child);
            goto failed;
        }
        if (dmabuf->offsets[i]+dmabuf->strides[i]*height >
         fd_get_size(dmabuf->fds[i])) {
            wl_resource_post_error(resource,
             ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
              "offset + stride * height goes out of dmabuf bounds");
            wl_resource_destroy(child);
            goto failed;
        }
        dmabuf->fds[i] = plane->fd;
        dmabuf->offsets[i] = plane->offset;
        dmabuf->strides[i] = plane->stride;
        dmabuf->modifiers[i] = plane->modifier;
    }

    if (dmabuf->buffer_dmabuf_events.create)
        dmabuf->buffer_dmabuf_events.create(dmabuf,
         dmabuf->buffer_dmabuf_events.user_data);

    if (buffer_id == 0) // `create` request
        zwp_linux_buffer_params_v1_send_created(resource, child);
    return;
failed:
    if (buffer_id == 0) // `create` request
        zwp_linux_buffer_params_v1_send_failed(resource);
        */
}

void Extensions::LinuxDMABuffer::Params::create(wl_client *client, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags)
{
    LLog::log("Create");
    create_(client, resource, 0, width, height, format, flags);
}

void Extensions::LinuxDMABuffer::Params::create_immed(wl_client *client, wl_resource *resource, UInt32 buffer_id, Int32 width, Int32 height, UInt32 format, UInt32 flags)
{
    LLog::log("Create immed");
    create_(client, resource, buffer_id, width, height, format, flags);
}
