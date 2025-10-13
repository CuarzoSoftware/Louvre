#include <CZ/Louvre/Protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GZwpLinuxDmaBufV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RZwpLinuxDmaBufFeedbackV1.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/LDMAFeedback.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Ream/RCore.h>

using namespace CZ::Protocols::LinuxDMABuf;

static const struct zwp_linux_dmabuf_feedback_v1_interface imp
{
    .destroy = &RZwpLinuxDmaBufFeedbackV1::destroy
};

RZwpLinuxDmaBufFeedbackV1::RZwpLinuxDmaBufFeedbackV1(
    GZwpLinuxDmaBufV1 *linuxDMABufRes,
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
    auto feedback { compositor()->backend()->defaultFeedback() };
    dev_t devId { feedback->mainDevice()->id() };
    wl_array dev {
        .size = sizeof(devId),
        .alloc = 0,
        .data = (void *)&devId,
    };

    mainDevice(&dev);
    formatTable(feedback->table()->fd(), feedback->table()->size());

    for (auto &tranche : feedback->tranches())
    {
        devId = tranche.device->id();
        trancheTargetDevice(&dev);
        trancheFlags(tranche.flags.get());

        wl_array formats {};
        formats.size = tranche.formatIdx().size() * sizeof(UInt16);
        formats.data = (void*)tranche.formatIdx().data();
        trancheFormats(&formats);
        trancheDone();
    }

    done();
}

/******************** REQUESTS ********************/

void RZwpLinuxDmaBufFeedbackV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RZwpLinuxDmaBufFeedbackV1::done() noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_done(resource());
}

void RZwpLinuxDmaBufFeedbackV1::formatTable(Int32 fd, UInt32 size) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_format_table(resource(), fd, size);
}

void RZwpLinuxDmaBufFeedbackV1::mainDevice(wl_array *devices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_main_device(resource(), devices);
}

void RZwpLinuxDmaBufFeedbackV1::trancheDone() noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_done(resource());
}

void RZwpLinuxDmaBufFeedbackV1::trancheTargetDevice(wl_array *devices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_target_device(resource(), devices);
}

void RZwpLinuxDmaBufFeedbackV1::trancheFormats(wl_array *indices) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_formats(resource(), indices);
}

void RZwpLinuxDmaBufFeedbackV1::trancheFlags(UInt32 flags) noexcept
{
    zwp_linux_dmabuf_feedback_v1_send_tranche_flags(resource(), flags);
}
