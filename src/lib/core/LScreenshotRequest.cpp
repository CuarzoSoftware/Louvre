#include <protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/LinuxDMABuf/LDMABuffer.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LOutputPrivate.h>
#include <LGLFramebuffer.h>
#include <LOutputMode.h>
#include <LUtils.h>
#include <LTime.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace Louvre;
using namespace Louvre::Protocols::ScreenCopy;

LClient *LScreenshotRequest::client() const noexcept
{
    return resource().client();
}

const LRect &LScreenshotRequest::rect() const noexcept
{
    return resource().rect();
}

void LScreenshotRequest::accept(bool accept) noexcept
{
    resource().m_stateFlags.setFlag(RScreenCopyFrame::Accepted, accept);
}

LScreenshotRequest::~LScreenshotRequest() noexcept
{
    if (resource().output())
        LVectorRemoveOneUnordered(resource().output()->imp()->screenshotRequests, this);
}

void LScreenshotRequest::copy(const LRegion &bufferRegion) noexcept
{
    if (wl_shm_buffer_get(resource().buffer()))
    {
        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(resource().buffer());
        wl_shm_buffer_begin_access(shm_buffer);
        UInt8 *pixels { static_cast<UInt8*>(wl_shm_buffer_get_data(shm_buffer)) };
        Int32 n;
        const LBox *box { bufferRegion.boxes(&n)};
        const GLenum format { static_cast<GLenum>(resource().output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra ? GL_BGRA : GL_RGBA) };
        const Int32 screenH { resource().output()->currentMode()->sizeB().h() };
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, wl_shm_buffer_get_width(shm_buffer));

        for (Int32 i = 0; i < n; i++)
        {
            glPixelStorei(GL_PACK_SKIP_PIXELS, box->x1 - resource().rectB().x());
            glPixelStorei(GL_PACK_SKIP_ROWS, box->y1 - resource().rectB().y());
            glReadPixels(box->x1,
                         screenH - box->y2,
                         box->x2 - box->x1,
                         box->y2 - box->y1,
                         format,
                         GL_UNSIGNED_BYTE,
                         pixels);
            box++;
        }
        wl_shm_buffer_end_access(shm_buffer);

        resource().flags(ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT);
    }
    else
    {
        LDMABuffer *dmaBuffer { static_cast<LDMABuffer*>(wl_resource_get_user_data(resource().buffer())) };
        UInt32 i { 0 };
        EGLAttrib attribs[19];
        attribs[i++] = EGL_WIDTH;
        attribs[i++] = dmaBuffer->planes()->width;
        attribs[i++] = EGL_HEIGHT;
        attribs[i++] = dmaBuffer->planes()->height;
        attribs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs[i++] = dmaBuffer->planes()->format;
        attribs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs[i++] = dmaBuffer->planes()->fds[0];
        attribs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs[i++] = dmaBuffer->planes()->offsets[0];
        attribs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs[i++] = dmaBuffer->planes()->strides[0];
        attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
        attribs[i++] = dmaBuffer->planes()->modifiers[0] & 0xFFFFFFFF;
        attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
        attribs[i++] = dmaBuffer->planes()->modifiers[0] >> 32;
        attribs[i++] = EGL_IMAGE_PRESERVED_KHR;
        attribs[i++] = EGL_TRUE;
        attribs[i++] = EGL_NONE;

        EGLImage image { eglCreateImage(compositor()->eglDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs) };

        if (image == EGL_NO_IMAGE)
        {
            resource().failed();
            return;
        }

        GLuint rb, fb;
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        compositor()->imp()->glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteFramebuffers(1, &fb);
            glDeleteRenderbuffers(1, &rb);
            eglDestroyImage(compositor()->eglDisplay(), image);
            resource().failed();
            return;
        }

        LGLFramebuffer glFb(
            fb,
            LSize((Int32)dmaBuffer->planes()->width, (Int32)dmaBuffer->planes()->height),
            resource().rectB().pos());

        LTexture *outputTexture { resource().output()->bufferTexture(resource().output()->currentBuffer()) };
        LPainter &p { *resource().output()->painter() };
        LFramebuffer *prevFb { p.boundFramebuffer() };
        glFinish();
        p.setAlpha(1.f);
        p.setColorFactor(1.f, 1.f, 1.f, 1.f);
        p.bindFramebuffer(&glFb);
        p.enableCustomTextureColor(false);
        p.bindTextureMode({
            .texture = outputTexture,
            .pos = LPoint(0, 0),
            .srcRect = LRectF(0, outputTexture->sizeB()),
            .dstSize = glFb.rect().size(),
            .srcTransform = LFramebuffer::Normal,
            .srcScale = 1.f,
        });
        glDisable(GL_BLEND);
        p.drawRegion(bufferRegion);
        p.bindFramebuffer(prevFb);
        glFinish();
        glDeleteFramebuffers(1, &fb);
        glDeleteRenderbuffers(1, &rb);
        eglDestroyImage(compositor()->eglDisplay(), image);
        resource().flags(0);
    }

    const timespec t { LTime::ns() };
    resource().damage(LRect(0, resource().rectB().size()));
    resource().ready(t.tv_sec >> 32, t.tv_sec & 0xffffffff, t.tv_nsec);
}
