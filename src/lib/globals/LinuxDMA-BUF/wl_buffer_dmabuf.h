#ifdef LOUVRE_DMA_ENABLE
#ifndef MY_WL_BUFFER_DMABUF_H
#define MY_WL_BUFFER_DMABUF_H

#include "wayland-util.h"
#include <EGL/egl.h>
#include <inttypes.h>
#include <stdbool.h>

struct bufstate
{
    EGLImage eglImage;
    struct gbm_bo *bo;
    uint32_t fb_id;
};

struct wl_buffer_dmabuf_data;

typedef void (*buffer_dmabuf_create_t)(struct wl_buffer_dmabuf_data *, void *);
typedef void (*buffer_dmabuf_destroy_t)(struct wl_buffer_dmabuf_data *, void *);

struct buffer_dmabuf_events
{
    buffer_dmabuf_create_t create;
    buffer_dmabuf_destroy_t destroy;
    void *user_data;
};

enum subsystem
{
    SUBSYSTEM_DRM,
    SUBSYSTEM_VULKAN,
    SUBSYSTEM_N
};

struct wl_buffer_dmabuf_data
{
    int32_t width;
    int32_t height;
    uint32_t format;
    uint32_t flags;

    uint32_t num_planes;
    int32_t *fds;
    uint32_t *offsets;
    uint32_t *strides;
    uint64_t *modifiers;

    struct buffer_dmabuf_events buffer_dmabuf_events;
/*
 * Subsystem-specific object for this buffer
 */
    void *subsystem_object[SUBSYSTEM_N];
};

struct wl_resource;

struct wl_buffer_dmabuf_data *wl_buffer_dmabuf_new(struct wl_resource *resource,
int32_t width, int32_t height, uint32_t format, uint32_t flags, uint32_t
num_planes, struct buffer_dmabuf_events buffer_dmabuf_events);

bool wl_buffer_is_dmabuf(struct wl_resource *);

int32_t wl_buffer_dmabuf_get_width(struct wl_resource *);
int32_t wl_buffer_dmabuf_get_height(struct wl_resource *);
uint32_t wl_buffer_dmabuf_get_format(struct wl_resource *);
uint32_t wl_buffer_dmabuf_get_num_planes(struct wl_resource *);
uint32_t *wl_buffer_dmabuf_get_offsets(struct wl_resource *);
uint32_t *wl_buffer_dmabuf_get_strides(struct wl_resource *);
uint64_t *wl_buffer_dmabuf_get_mods(struct wl_resource *);

void *wl_buffer_dmabuf_get_subsystem_object(struct wl_resource *, enum subsystem);

















struct zwp_linux_buffer_params_v1_data {
    bool already_used;
    struct wl_list plane_list;
    struct buffer_dmabuf_events buffer_dmabuf_events;
};

#endif
#endif
