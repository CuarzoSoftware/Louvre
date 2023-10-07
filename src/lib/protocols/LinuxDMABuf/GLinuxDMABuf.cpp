#include <protocols/LinuxDMABuf/private/GLinuxDMABufPrivate.h>
#include <protocols/LinuxDMABuf/linux-dmabuf-unstable-v1.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::LinuxDMABuf;

GLinuxDMABuf::GLinuxDMABuf
(
    wl_client *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation,
        destroy
    )
{
    m_imp = new GLinuxDMABufPrivate();
    this->client()->imp()->linuxDMABufGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->linuxDMABufGlobals.end());

    if (version < 3)
    {
        Int64 prevFormat = -1;

        for (LDMAFormat *dmaFormat : *compositor()->imp()->graphicBackend->getDMAFormats())
        {
            if (dmaFormat->format != prevFormat)
            {
                format(dmaFormat->format);
                prevFormat = dmaFormat->format;
            }
        }
    }
    else
    {
        for (LDMAFormat *dmaFormat : *compositor()->imp()->graphicBackend->getDMAFormats())
            modifier(dmaFormat->format,
                     dmaFormat->modifier >> 32,
                     dmaFormat->modifier & 0xffffffff);
    }
}

GLinuxDMABuf::~GLinuxDMABuf()
{
    client()->imp()->linuxDMABufGlobals.erase(imp()->clientLink);
    delete m_imp;
}

bool GLinuxDMABuf::format(UInt32 format)
{
    zwp_linux_dmabuf_v1_send_format(resource(), format);
    return true;
}

bool GLinuxDMABuf::modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo)
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
