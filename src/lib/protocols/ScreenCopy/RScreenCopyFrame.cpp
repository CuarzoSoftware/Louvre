#include <protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>

using namespace Louvre::Protocols::ScreenCopy;

static const struct zwlr_screencopy_frame_v1_interface imp
{
    .copy = &RScreenCopyFrame::copy,
    .destroy = &RScreenCopyFrame::destroy,

#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    .copy_with_damage = &RScreenCopyFrame::copy_with_damage,
#else
    .copy_with_damage = NULL
#endif
};

RScreenCopyFrame::RScreenCopyFrame
    (
        Wayland::GOutput *outputRes,
        bool overlayCursor,
        const LRect &region,
        UInt32 id,
        Int32 version
    ) noexcept
    :LResource
    (
        outputRes->client(),
        &zwlr_screencopy_frame_v1_interface,
        version,
        id,
        &imp
    ),
    m_outputRes(outputRes),
    m_frame(*this),
    m_region(region),
    m_overlayCursor(overlayCursor)
{
    m_bufferContainer.onBufferDestroyListener.notify = [](wl_listener *listener, void *)
    {
        BufferContainer *bufferContainer { (BufferContainer *)listener };
        bufferContainer->m_buffer = nullptr;
    };

    /*
    LRegion bufferRegion(region);
    bufferRegion.transform(outputRes->output()->size(), LFramebuffer::requiredTransform(LFramebuffer::Normal, outputRes->output()->transform()));*/

    if (outputRes->output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra)
    {
        buffer(WL_SHM_FORMAT_XRGB8888,
               outputRes->output()->realBufferSize(),
               outputRes->output()->realBufferSize().w() * 4);
    }
    else
    {
        buffer(WL_SHM_FORMAT_XBGR8888,
               outputRes->output()->realBufferSize(),
               outputRes->output()->realBufferSize().w() * 4);
    }
}

RScreenCopyFrame::~RScreenCopyFrame() noexcept
{
    if (m_bufferContainer.m_buffer)
        wl_list_remove(&m_bufferContainer.onBufferDestroyListener.link);
}

/******************** REQUESTS ********************/

void RScreenCopyFrame::copy(wl_client */*client*/, wl_resource *resource, wl_resource *buffer) noexcept
{
    auto &res { *static_cast<RScreenCopyFrame*>(wl_resource_get_user_data(resource)) };

    if (res.m_used)
    {
        wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_ALREADY_USED, "The object has already been used to copy a wl_buffer.");
        return;
    }

    res.m_used = true;

    if (wl_shm_buffer_get(buffer))
    {
        wl_shm_buffer *shm_buffer { wl_shm_buffer_get(buffer) };

        if ((res.outputRes()->output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra && wl_shm_buffer_get_format(shm_buffer) != WL_SHM_FORMAT_XRGB8888) ||
            (!res.outputRes()->output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra && wl_shm_buffer_get_format(shm_buffer) != WL_SHM_FORMAT_XBGR8888))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer format.");
            return;
        }

        if (res.m_sentBufferStride != wl_shm_buffer_get_stride(shm_buffer))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer stride.");
            return;
        }

        if (res.m_sentBufferSize.w() != wl_shm_buffer_get_width(shm_buffer) || res.m_sentBufferSize.h() != wl_shm_buffer_get_height(shm_buffer))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer size.");
            return;
        }
    }
    else
    {
        wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer type.");
        return;
    }

    res.m_waitForDamage = false;
    res.m_bufferContainer.m_buffer = buffer;
    wl_resource_add_destroy_listener(buffer, &res.m_bufferContainer.onBufferDestroyListener);
    res.outputRes()->output()->imp()->screenCopyFrames.emplace_back(&res.m_frame);
}

void RScreenCopyFrame::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
void RScreenCopyFrame::copy_with_damage(wl_client */*client*/, wl_resource *resource, wl_resource *buffer) noexcept
{

}
#endif

/******************** EVENTS ********************/

void RScreenCopyFrame::buffer(UInt32 shmFormat, const LSize &size, UInt32 stride) noexcept
{
    m_sentBufferSize = outputRes()->output()->realBufferSize();
    m_sentBufferStride = stride;
    zwlr_screencopy_frame_v1_send_buffer(resource(), shmFormat, size.w(), size.h(), stride);
}

void RScreenCopyFrame::flags(UInt32 flags) noexcept
{
    zwlr_screencopy_frame_v1_send_flags(resource(), flags);
}

void RScreenCopyFrame::ready(UInt32 tvSecHi, UInt32 tvSecLow, UInt32 tvNsec) noexcept
{
    if (!m_handled)
    {
        zwlr_screencopy_frame_v1_send_ready(resource(), tvSecHi, tvSecLow, tvNsec);
        m_handled = true;
    }
}

void RScreenCopyFrame::failed() noexcept
{
    if (!m_handled)
    {
        zwlr_screencopy_frame_v1_send_failed(resource());
        m_handled = true;
    }
}

bool RScreenCopyFrame::damage(const LRect &rect) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        zwlr_screencopy_frame_v1_send_damage(resource(), rect.x(), rect.y(), rect.w(), rect.h());
        return true;
    }
#else
    L_UNUSED(rect)
#endif
    return false;
}

bool RScreenCopyFrame::linuxDMABuf(UInt32 format, const LSize &size) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        zwlr_screencopy_frame_v1_send_linux_dmabuf(resource(), format, size.w(), size.h());
        return true;
    }
#else
    L_UNUSED(format)
    L_UNUSED(size)
#endif
    return false;
}

bool RScreenCopyFrame::bufferDone() noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        zwlr_screencopy_frame_v1_send_buffer_done(resource());
        return true;
    }
#endif
    return false;
}
