#include <CZ/Louvre/Protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GLinuxDMABuf.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RLinuxBufferParams.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RLinuxDMABufFeedback.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::LinuxDMABuf;

static const struct zwp_linux_dmabuf_v1_interface imp
{
    .destroy = &GLinuxDMABuf::destroy,
    .create_params = &GLinuxDMABuf::create_params,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
    .get_default_feedback = &GLinuxDMABuf::get_default_feedback,
    .get_surface_feedback = &GLinuxDMABuf::get_surface_feedback
#else
    .get_default_feedback = NULL,
    .get_surface_feedback = NULL
#endif
};

void GLinuxDMABuf::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GLinuxDMABuf(client, version, id);
}

Int32 GLinuxDMABuf::maxVersion() noexcept
{
    return LOUVRE_LINUX_DMA_BUF_VERSION;
}

const wl_interface *GLinuxDMABuf::interface() noexcept
{
    return &zwp_linux_dmabuf_v1_interface;
}

GLinuxDMABuf::GLinuxDMABuf(
    wl_client *client,
    Int32 version,
    UInt32 id
) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->linuxDMABufGlobals.emplace_back(this);

    if (version < 3)
    {
        Int64 prevFormat { -1 };

        for (const LDMAFormat &dmaFormat : *compositor()->imp()->graphicBackend->backendGetDMAFormats())
        {
            if (dmaFormat.format != prevFormat)
            {
                format(dmaFormat.format);
                prevFormat = dmaFormat.format;
            }
        }
    }
    else if (version == 3)
    {
        for (const LDMAFormat &dmaFormat : *compositor()->imp()->graphicBackend->backendGetDMAFormats())
            modifier(dmaFormat.format,
                     dmaFormat.modifier >> 32,
                     dmaFormat.modifier & 0xffffffff);
    }
}

GLinuxDMABuf::~GLinuxDMABuf() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->linuxDMABufGlobals, this);
}

/******************** REQUESTS ********************/

void GLinuxDMABuf::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GLinuxDMABuf::create_params(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RLinuxBufferParams(static_cast<GLinuxDMABuf*>(wl_resource_get_user_data(resource)), id);
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
void GLinuxDMABuf::get_default_feedback(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    new RLinuxDMABufFeedback(static_cast<GLinuxDMABuf*>(wl_resource_get_user_data(resource)), id);
}
void GLinuxDMABuf::get_surface_feedback(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource */*surface*/)
{
    new RLinuxDMABufFeedback(static_cast<GLinuxDMABuf*>(wl_resource_get_user_data(resource)), id);
}
#endif

/******************** EVENTS ********************/

void GLinuxDMABuf::format(UInt32 format) noexcept
{
    zwp_linux_dmabuf_v1_send_format(resource(), format);
}

bool GLinuxDMABuf::modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo) noexcept
{
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 3
    if (version() >= 3)
    {
        zwp_linux_dmabuf_v1_send_modifier(resource(),
                                          format,
                                          mod_hi,
                                          mod_lo);
        return true;
    }
#endif

    L_UNUSED(format);
    L_UNUSED(mod_hi);
    L_UNUSED(mod_lo);
    return false;
}
