#include <protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <protocols/ScreenCopy/GScreenCopyManager.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/LinuxDMABuf/LDMABuffer.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>
#include <LOutputMode.h>
#include <LUtils.h>

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
        GScreenCopyManager *screenCopyManagerRes,
        LOutput *output,
        bool overlayCursor,
        const LRect &region,
        UInt32 id,
        Int32 version
    ) noexcept
    :LResource
    (
        screenCopyManagerRes->client(),
        &zwlr_screencopy_frame_v1_interface,
        version,
        id,
        &imp
    ),
    m_output(output),
    m_screenCopyManagerRes(screenCopyManagerRes),
    m_frame(*this),
    m_rect(region)
{
    if (!output)
    {
        buffer(WL_SHM_FORMAT_XRGB8888, LSize(1), 4);
        bufferDone();
        failed();
        return;
    }

    m_stateFlags.setFlag(CompositeCursor, overlayCursor);

    m_bufferContainer.onDestroy.notify = [](wl_listener *listener, void *)
    {
        BufferContainer *bufferContainer { (BufferContainer *)listener };
        bufferContainer->buffer = nullptr;
    };

    m_initOutputModeSize = output->currentMode()->sizeB();
    m_initOutputSize = output->size();
    m_initOutputTransform = output->transform();

    auto &damage { screenCopyManagerRes->damage[output] };

    if (damage.firstFrame)
    {
        damage.damage.addRect(0, m_initOutputModeSize);
        damage.firstFrame = false;
    }

    LSizeF scale;

    if (Louvre::is90Transform(output->transform()))
    {
        scale.setW(Float32(m_initOutputModeSize.w())/Float32(m_initOutputSize.h()));
        scale.setH(Float32(m_initOutputModeSize.h())/Float32(m_initOutputSize.w()));
    }
    else
    {
        scale.setW(Float32(m_initOutputModeSize.w())/Float32(m_initOutputSize.w()));
        scale.setH(Float32(m_initOutputModeSize.h())/Float32(m_initOutputSize.h()));
    }

    LRegion bufferRegion(m_rect);
    bufferRegion.transform(m_initOutputSize, output->transform());
    bufferRegion.multiply(scale.x(), scale.y());

    bufferRegion.clip(0, m_initOutputModeSize);
    const LBox &extents { bufferRegion.extents() };
    m_rectB.setX(extents.x1);
    m_rectB.setY(extents.y1);
    m_rectB.setW(extents.x2 - extents.x1);
    m_rectB.setH(extents.y2 - extents.y1);
    m_stride = m_rectB.w() * 4;

    if (output->painter()->imp()->openGLExtensions.EXT_read_format_bgra)
    {
        buffer(WL_SHM_FORMAT_XRGB8888,
               m_rectB.size(),
               m_stride);
    }
    else
    {
        buffer(WL_SHM_FORMAT_XBGR8888,
               m_rectB.size(),
               m_stride);
    }

    LTexture *outputTexture;

    if ((outputTexture = output->bufferTexture(0)) && output->painter()->imp()->openGLExtensions.OES_EGL_image)
        linuxDMABuf(outputTexture->format(), m_rectB.size());

    bufferDone();
}

RScreenCopyFrame::~RScreenCopyFrame() noexcept
{
    if (m_bufferContainer.buffer)
        wl_list_remove(&m_bufferContainer.onDestroy.link);

    if (output())
        LVectorRemoveOneUnordered(output()->imp()->screenshotRequests, &m_frame);
}

void RScreenCopyFrame::copyCommon(wl_resource *resource, wl_resource *buffer, bool waitForDamage) noexcept
{
    auto &res { *static_cast<RScreenCopyFrame*>(wl_resource_get_user_data(resource)) };

    if (!res.output())
    {
        res.failed();
        return;
    }

    if (res.alreadyUsed())
    {
        wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_ALREADY_USED, "The object has already been used to copy a wl_buffer.");
        return;
    }

    res.m_stateFlags.setFlag(WaitForDamage, waitForDamage);
    res.m_stateFlags.add(AlreadyUsed);

    if (wl_shm_buffer_get(buffer))
    {
        wl_shm_buffer *shmBuffer { wl_shm_buffer_get(buffer) };

        if ((res.output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra && wl_shm_buffer_get_format(shmBuffer) != WL_SHM_FORMAT_XRGB8888) ||
            (!res.output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra && wl_shm_buffer_get_format(shmBuffer) != WL_SHM_FORMAT_XBGR8888))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer format.");
            return;
        }

        if (res.m_stride != wl_shm_buffer_get_stride(shmBuffer))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer stride.");
            return;
        }

        if (res.rectB().w() != wl_shm_buffer_get_width(shmBuffer) || res.rectB().h() != wl_shm_buffer_get_height(shmBuffer))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer size.");
            return;
        }
    }
    else if (LDMABuffer::isDMABuffer(buffer))
    {
        LDMABuffer *dmaBuffer { static_cast<LDMABuffer*>(wl_resource_get_user_data(buffer)) };

        if (!res.output()->bufferTexture(0))
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Linux DMA buffers are not supported.");
            return;
        }

        if (dmaBuffer->texture())
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Buffer already used by surfaces.");
            return;
        }

        if (res.output()->bufferTexture(0)->format() != dmaBuffer->planes()->format)
        {
            wl_resource_post_error(resource, ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer format.");
            return;
        }

        if (static_cast<Int32>(dmaBuffer->planes()->width) != res.rectB().w() || static_cast<Int32>(dmaBuffer->planes()->height) != res.rectB().h())
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

    res.m_bufferContainer.buffer = buffer;
    wl_resource_add_destroy_listener(buffer, &res.m_bufferContainer.onDestroy);
    res.output()->imp()->screenshotRequests.emplace_back(&res.m_frame);
}

/******************** REQUESTS ********************/

void RScreenCopyFrame::copy(wl_client */*client*/, wl_resource *resource, wl_resource *buffer) noexcept
{
    RScreenCopyFrame::copyCommon(resource, buffer, false);
}

void RScreenCopyFrame::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
void RScreenCopyFrame::copy_with_damage(wl_client */*client*/, wl_resource *resource, wl_resource *buffer) noexcept
{
    RScreenCopyFrame::copyCommon(resource, buffer, true);
}
#endif

/******************** EVENTS ********************/

void RScreenCopyFrame::buffer(UInt32 shmFormat, const LSize &size, UInt32 stride) noexcept
{
    zwlr_screencopy_frame_v1_send_buffer(resource(), shmFormat, size.w(), size.h(), stride);
}

void RScreenCopyFrame::flags(UInt32 flags) noexcept
{
    zwlr_screencopy_frame_v1_send_flags(resource(), flags);
}

void RScreenCopyFrame::ready(const timespec &time) noexcept
{
    zwlr_screencopy_frame_v1_send_ready(resource(),
                                        time.tv_sec >> 32,
                                        time.tv_sec & 0xffffffff,
                                        time.tv_nsec);
}

void RScreenCopyFrame::failed() noexcept
{
    zwlr_screencopy_frame_v1_send_failed(resource());
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

bool RScreenCopyFrame::damage(const LRegion &region) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        Int32 n;
        const LBox *box { region.boxes(&n) };

        while (n > 0)
        {
            zwlr_screencopy_frame_v1_send_damage(
                resource(),
                box->x1,
                box->y1,
                box->x2 - box->x1,
                box->y2 - box->y1);
            box++;
            n--;
        }
        return true;
    }
#else
    L_UNUSED(region)
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
