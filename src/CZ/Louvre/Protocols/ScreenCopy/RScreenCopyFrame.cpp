#include <CZ/Louvre/Protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <CZ/Louvre/Protocols/ScreenCopy/GScreenCopyManager.h>
#include <CZ/Louvre/Protocols/ScreenCopy/RScreenCopyFrame.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>
#include <CZ/Utils/CZRegionUtils.h>
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
        const SkIRect &region,
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
        buffer(WL_SHM_FORMAT_XRGB8888, SkISize(1, 1), 4);
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
        damage.damage.op(SkIRect::MakeSize(m_initOutputModeSize), SkRegion::Op::kUnion_Op);
        damage.firstFrame = false;
    }

    SkSize scale;

    if (CZ::Is90Transform(output->transform()))
    {
        scale.fWidth = Float32(m_initOutputModeSize.width())/Float32(m_initOutputSize.height());
        scale.fHeight = Float32(m_initOutputModeSize.height())/Float32(m_initOutputSize.width());
    }
    else
    {
        scale.fWidth = Float32(m_initOutputModeSize.width())/Float32(m_initOutputSize.width());
        scale.fHeight = Float32(m_initOutputModeSize.height())/Float32(m_initOutputSize.height());
    }

    SkRegion bufferRegion(m_rect);
    CZRegionUtils::ApplyTransform(bufferRegion, m_initOutputSize, output->transform());
    CZRegionUtils::Scale(bufferRegion, scale.width(), scale.height());
    bufferRegion.op(SkIRect::MakeSize(m_initOutputModeSize), SkRegion::Op::kIntersect_Op);

    m_rectB = bufferRegion.getBounds();
    m_stride = m_rectB.width() * 4;

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
        res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_ALREADY_USED, "The object has already been used to copy a wl_buffer.");
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
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer format.");
            return;
        }

        if (res.m_stride != wl_shm_buffer_get_stride(shmBuffer))
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer stride.");
            return;
        }

        if (res.rectB().width() != wl_shm_buffer_get_width(shmBuffer) || res.rectB().height() != wl_shm_buffer_get_height(shmBuffer))
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer size.");
            return;
        }
    }
    else if (LDMABuffer::isDMABuffer(buffer))
    {
        LDMABuffer *dmaBuffer { static_cast<LDMABuffer*>(wl_resource_get_user_data(buffer)) };

        if (!res.output()->bufferTexture(0))
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Linux DMA buffers are not supported.");
            return;
        }

        if (dmaBuffer->texture())
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Buffer already used by surfaces.");
            return;
        }

        if (res.output()->bufferTexture(0)->format() != dmaBuffer->planes()->format)
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer format.");
            return;
        }

        if (static_cast<Int32>(dmaBuffer->planes()->width) != res.rectB().width() || static_cast<Int32>(dmaBuffer->planes()->height) != res.rectB().height())
        {
            res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer size.");
            return;
        }
    }
    else
    {
        res.postError(ZWLR_SCREENCOPY_FRAME_V1_ERROR_INVALID_BUFFER, "Invalid buffer type.");
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

void RScreenCopyFrame::buffer(UInt32 shmFormat, const SkISize &size, UInt32 stride) noexcept
{
    zwlr_screencopy_frame_v1_send_buffer(resource(), shmFormat, size.width(), size.height(), stride);
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

bool RScreenCopyFrame::damage(const SkIRect &rect) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        zwlr_screencopy_frame_v1_send_damage(resource(), rect.x(), rect.y(), rect.width(), rect.height());
        return true;
    }
#else
    L_UNUSED(rect)
#endif
    return false;
}

bool RScreenCopyFrame::damage(const SkRegion &region) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        SkRegion::Iterator it(region);

        while (!it.done())
        {
            zwlr_screencopy_frame_v1_send_damage(
                resource(),
                it.rect().x(),
                it.rect().y(),
                it.rect().width(),
                it.rect().height());
            it.next();
        }

        return true;
    }
#else
    L_UNUSED(region)
#endif
    return false;
}

bool RScreenCopyFrame::linuxDMABuf(UInt32 format, const SkISize &size) noexcept
{
#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        zwlr_screencopy_frame_v1_send_linux_dmabuf(resource(), format, size.width(), size.height());
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
