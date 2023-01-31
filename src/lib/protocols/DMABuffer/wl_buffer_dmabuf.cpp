#include "wl_buffer_dmabuf.h"

#ifdef lala
#define _POSIX_C_SOURCE 200809L

#include <wayland-server-protocol.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface impl = {destroy};

static void free_data(struct wl_resource *resource)
{
    struct wl_buffer_dmabuf_data *dmabuf = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
/*
 * buffer_dmabuf will be destroyed, notify subsystems for cleaning
 */
    if (dmabuf->buffer_dmabuf_events.destroy)
        dmabuf->buffer_dmabuf_events.destroy(dmabuf,
         dmabuf->buffer_dmabuf_events.user_data);

    for (size_t i=0; i<dmabuf->num_planes; i++)
        close(dmabuf->fds[i]);
    free(dmabuf->fds);
    free(dmabuf->offsets);
    free(dmabuf->strides);
    free(dmabuf->modifiers);
    free(dmabuf);
}

struct wl_buffer_dmabuf_data *wl_buffer_dmabuf_new(struct wl_resource *resource,
int32_t width, int32_t height, uint32_t format, uint32_t flags, uint32_t
num_planes, struct buffer_dmabuf_events buffer_dmabuf_events) {
    struct wl_buffer_dmabuf_data *dmabuf = (struct wl_buffer_dmabuf_data *)malloc(sizeof(*dmabuf));
    dmabuf->width = width;
    dmabuf->height = height;
    dmabuf->format = format;
    dmabuf->flags = flags;
    dmabuf->num_planes = num_planes;
    dmabuf->fds = (int32_t*)malloc(num_planes*sizeof(int32_t));
    dmabuf->offsets = (uint32_t*)malloc(num_planes*sizeof(uint32_t));
    dmabuf->strides = (uint32_t*)malloc(num_planes*sizeof(uint32_t));
    dmabuf->modifiers = (uint64_t*)malloc(num_planes*sizeof(uint64_t));
    dmabuf->buffer_dmabuf_events = buffer_dmabuf_events;
    memset(dmabuf->subsystem_object, 0, sizeof(dmabuf->subsystem_object));
    wl_resource_set_implementation(resource, &impl, dmabuf, free_data);
    return dmabuf;
}

bool wl_buffer_is_dmabuf(struct wl_resource *resource) {
    return wl_resource_instance_of(resource, &wl_buffer_interface, &impl);
}

int32_t wl_buffer_dmabuf_get_width(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->width;
}

int32_t wl_buffer_dmabuf_get_height(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->height;
}

uint32_t wl_buffer_dmabuf_get_format(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->format;
}

uint32_t wl_buffer_dmabuf_get_num_planes(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->num_planes;
}

uint32_t *wl_buffer_dmabuf_get_offsets(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->offsets;
}

uint32_t *wl_buffer_dmabuf_get_strides(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->strides;
}

uint64_t *wl_buffer_dmabuf_get_mods(struct wl_resource *resource) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->modifiers;
}

void *wl_buffer_dmabuf_get_subsystem_object(struct wl_resource *resource, enum
subsystem subsystem) {
    struct wl_buffer_dmabuf_data *data = (struct wl_buffer_dmabuf_data *)wl_resource_get_user_data(resource);
    return data->subsystem_object[subsystem];
}
#endif
