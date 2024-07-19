#include <protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <protocols/LinuxDMABuf/GLinuxDMABuf.h>
#include <protocols/LinuxDMABuf/RLinuxDMABufFeedback.h>
#include <private/LCompositorPrivate.h>

using namespace Louvre::Protocols::LinuxDMABuf;

static const struct zwp_linux_dmabuf_feedback_v1_interface imp
{
    .destroy = &RLinuxDMABufFeedback::destroy
};

RLinuxDMABufFeedback::RLinuxDMABufFeedback(
    GLinuxDMABuf *linuxDMABufRes,
    UInt32 id
    ) noexcept
    :LResource
    (
        linuxDMABufRes->client(),
        &zwp_linux_dmabuf_feedback_v1_interface,
        linuxDMABufRes->version(),
        id,
        &imp
    )
{
    auto &feedback { compositor()->imp()->dmaFeedback };

    if (feedback.tableFd >= 0)
    {
        wl_array dev {
            .size = sizeof(feedback.device),
            .alloc = 0,
            .data = (void *)&feedback.device,
        };

        mainDevice(&dev);
        formatTable(feedback.tableFd, feedback.tableSize);

        if (!compositor()->imp()->graphicBackend->backendGetScanoutDMAFormats()->empty())
        {
            trancheTargetDevice(&dev);
            trancheFlags(ZWP_LINUX_DMABUF_FEEDBACK_V1_TRANCHE_FLAGS_SCANOUT);
            trancheFormats(&feedback.scanoutIndices);
            trancheDone();
        }

        trancheTargetDevice(&dev);
        trancheFlags(0);
        trancheFormats(&feedback.formatIndices);
        trancheDone();
    }

    done();
}

/******************** REQUESTS ********************/

void RLinuxDMABufFeedback::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RLinuxDMABufFeedback::done() noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_done(resource());
}

void RLinuxDMABufFeedback::formatTable(Int32 fd, UInt32 size) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_format_table(resource(), fd, size);
}

void RLinuxDMABufFeedback::mainDevice(wl_array *devices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_main_device(resource(), devices);
}

void RLinuxDMABufFeedback::trancheDone() noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_done(resource());
}

void RLinuxDMABufFeedback::trancheTargetDevice(wl_array *devices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_target_device(resource(), devices);
}

void RLinuxDMABufFeedback::trancheFormats(wl_array *indices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_formats(resource(), indices);
}

void RLinuxDMABufFeedback::trancheFlags(UInt32 flags) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_flags(resource(), flags);
}

/******************** EVENTS ********************/

