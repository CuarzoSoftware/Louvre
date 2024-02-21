#include <protocols/LinuxDMABuf/private/GLinuxDMABufPrivate.h>
#include <protocols/LinuxDMABuf/private/RLinuxBufferParamsPrivate.h>
#include <protocols/LinuxDMABuf/linux-dmabuf-unstable-v1.h>

static struct zwp_linux_dmabuf_v1_interface zwp_linux_dmabuf_v1_implementation =
{
    .destroy = &GLinuxDMABuf::GLinuxDMABufPrivate::destroy,
    .create_params = &GLinuxDMABuf::GLinuxDMABufPrivate::create_params,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
    .get_default_feedback = &GLinuxDMABuf::GLinuxDMABufPrivate::get_default_feedback,
    .get_surface_feedback = &GLinuxDMABuf::GLinuxDMABufPrivate::get_surface_feedback
#else
    .get_default_feedback = NULL,
    .get_surface_feedback = NULL
#endif
};

void GLinuxDMABuf::GLinuxDMABufPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    new GLinuxDMABuf(client,
                     &zwp_linux_dmabuf_v1_interface,
                     version,
                     id,
                     &zwp_linux_dmabuf_v1_implementation);
}

void GLinuxDMABuf::GLinuxDMABufPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GLinuxDMABuf::GLinuxDMABufPrivate::create_params(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    GLinuxDMABuf *gLinuxDMABuf = (GLinuxDMABuf*)wl_resource_get_user_data(resource);
    new RLinuxBufferParams(gLinuxDMABuf, id);
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
void GLinuxDMABuf::GLinuxDMABufPrivate::get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id)
{

}

void GLinuxDMABuf::GLinuxDMABufPrivate::get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{

}
#endif
