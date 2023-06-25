#include <protocols/LinuxDMABuf/private/RLinuxBufferParamsPrivate.h>
#include <protocols/LinuxDMABuf/private/GLinuxDMABufPrivate.h>
#include <protocols/LinuxDMABuf/linux-dmabuf-unstable-v1.h>

using namespace Louvre;
using namespace Louvre::Protocols::LinuxDMABuf;

static struct zwp_linux_buffer_params_v1_interface zwp_linux_buffer_params_v1_implementation
{
    .destroy = &RLinuxBufferParams::RLinuxBufferParamsPrivate::destroy,
    .add = &RLinuxBufferParams::RLinuxBufferParamsPrivate::add,
    .create = &RLinuxBufferParams::RLinuxBufferParamsPrivate::create,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
    .create_immed = &RLinuxBufferParams::RLinuxBufferParamsPrivate::create_immed
#endif
};

RLinuxBufferParams::RLinuxBufferParams
(
    GLinuxDMABuf *gLinuxDMABuf,
    UInt32 id
)
    :LResource
    (
        gLinuxDMABuf->client(),
        &zwp_linux_buffer_params_v1_interface,
        gLinuxDMABuf->version(),
        id,
        &zwp_linux_buffer_params_v1_implementation,
        &RLinuxBufferParams::RLinuxBufferParamsPrivate::resource_destroy
    )
{
    m_imp = new RLinuxBufferParamsPrivate();
    imp()->planes = new LDMAPlanes();
}

RLinuxBufferParams::~RLinuxBufferParams()
{
    if (planes())
        delete imp()->planes;

    delete m_imp;
}

const LDMAPlanes *RLinuxBufferParams::planes() const
{
    return imp()->planes;
}

bool RLinuxBufferParams::created(wl_resource *buffer)
{
    zwp_linux_buffer_params_v1_send_created(resource(), buffer);
    return true;
}

bool RLinuxBufferParams::failed()
{
    zwp_linux_buffer_params_v1_send_failed(resource());
    return true;
}
