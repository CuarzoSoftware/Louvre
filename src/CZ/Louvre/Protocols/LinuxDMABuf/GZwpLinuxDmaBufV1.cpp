#include <CZ/Louvre/Protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GZwpLinuxDmaBufV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RZwpLinuxBufferParamsV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RZwpLinuxDmaBufFeedbackV1.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/LDMAFeedback.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::LinuxDMABuf;

static const struct zwp_linux_dmabuf_v1_interface imp
{
    .destroy = &GZwpLinuxDmaBufV1::destroy,
    .create_params = &GZwpLinuxDmaBufV1::create_params,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
    .get_default_feedback = &GZwpLinuxDmaBufV1::get_default_feedback,
    .get_surface_feedback = &GZwpLinuxDmaBufV1::get_surface_feedback
#else
    .get_default_feedback = NULL,
    .get_surface_feedback = NULL
#endif
};

LGLOBAL_INTERFACE_IMP(GZwpLinuxDmaBufV1, LOUVRE_LINUX_DMA_BUF_VERSION, zwp_linux_dmabuf_v1_interface)

bool GZwpLinuxDmaBufV1::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (!compositor()->backend()->defaultFeedback())
    {
        LLog(CZWarning, CZLN, "Failed to create {} global (no DMA formats supported by the current backend)", Interface()->name);
        return false;
    }

    if (compositor()->wellKnownGlobals.LinuxDMABuf)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.LinuxDMABuf;
    return true;
}

GZwpLinuxDmaBufV1::GZwpLinuxDmaBufV1(
    wl_client *client,
    Int32 version,
    UInt32 id
)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->linuxDMABufGlobals.emplace_back(this);

    auto feedback { compositor()->backend()->defaultFeedback() };

    assert(feedback && "Do not advertise GZwpLinuxDmaBufV1 support if the backend doesn't provide a default feedback table. See LCompositor::createGlobalsRequest.");

    if (version < 3)
    {
        for (auto &fmt : feedback->tranches().front().formatSet.formats())
            if (fmt.modifiers().contains(DRM_FORMAT_MOD_INVALID) || fmt.modifiers().contains(DRM_FORMAT_MOD_LINEAR))
                format(fmt.format());
    }
    else if (version == 3)
    {
        for (auto &fmt : feedback->tranches().front().formatSet.formats())
            for (auto mod : fmt.modifiers())
                modifier(fmt.format(), mod >> 32, mod & 0xffffffff);
    }
}

GZwpLinuxDmaBufV1::~GZwpLinuxDmaBufV1() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->linuxDMABufGlobals, this);
}

/******************** REQUESTS ********************/

void GZwpLinuxDmaBufV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GZwpLinuxDmaBufV1::create_params(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RZwpLinuxBufferParamsV1(static_cast<GZwpLinuxDmaBufV1*>(wl_resource_get_user_data(resource)), id);
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
void GZwpLinuxDmaBufV1::get_default_feedback(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    new RZwpLinuxDmaBufFeedbackV1(static_cast<GZwpLinuxDmaBufV1*>(wl_resource_get_user_data(resource)), id);
}
void GZwpLinuxDmaBufV1::get_surface_feedback(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource */*surface*/)
{
    new RZwpLinuxDmaBufFeedbackV1(static_cast<GZwpLinuxDmaBufV1*>(wl_resource_get_user_data(resource)), id);
}
#endif

/******************** EVENTS ********************/

void GZwpLinuxDmaBufV1::format(UInt32 format) noexcept
{
    zwp_linux_dmabuf_v1_send_format(resource(), format);
}

bool GZwpLinuxDmaBufV1::modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo) noexcept
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
