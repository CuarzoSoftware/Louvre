#include <CZ/Louvre/Protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <CZ/Louvre/Protocols/ScreenCopy/RScreenCopyFrame.h>
#include <CZ/Louvre/Protocols/ScreenCopy/GScreenCopyManager.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/LFramebufferWrapper.h>
#include <CZ/Louvre/LOutputMode.h>
#include <CZ/Louvre/LUtils.h>
#include <CZ/Louvre/LTime.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace Louvre;
using namespace Louvre::Protocols::ScreenCopy;

LClient *LScreenshotRequest::client() const noexcept
{
    return resource().client();
}

const SkIRect &LScreenshotRequest::rect() const noexcept
{
    return resource().rect();
}

void LScreenshotRequest::accept(bool accept) noexcept
{
    resource().m_stateFlags.setFlag(RScreenCopyFrame::Accepted, accept);
}

Int8 LScreenshotRequest::copy() noexcept
{
    SkRegion damage;

    if (wl_shm_buffer_get(resource().buffer()))
    {
        if (resource().screenCopyManagerRes())
        {
            auto &outputDamage { resource().screenCopyManagerRes()->damage[resource().output()] };
            outputDamage.damage.op(resource().rectB(), SkRegion::Op::kIntersect_Op);

            // No damage, wait...
            if (resource().waitForDamage() && outputDamage.damage.isEmpty())
                return 0;

            damage = std::move(outputDamage.damage);
        }
        else
        {
            // No damage tracking
            damage.setRect(resource().rectB());
        }

        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(resource().buffer());
        wl_shm_buffer_begin_access(shm_buffer);
        UInt8 *pixels { static_cast<UInt8*>(wl_shm_buffer_get_data(shm_buffer)) };
        const GLenum format { static_cast<GLenum>(resource().output()->painter()->imp()->openGLExtensions.EXT_read_format_bgra ? GL_BGRA : GL_RGBA) };
        const Int32 screenH { resource().output()->currentMode()->sizeB().height() };
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, wl_shm_buffer_get_width(shm_buffer));
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);

        glReadPixels(resource().rectB().x(),
                     screenH - (resource().rectB().y() + resource().rectB().height()),
                     resource().rectB().width(),
                     resource().rectB().height(),
                     format,
                     GL_UNSIGNED_BYTE,
                     pixels);

        wl_shm_buffer_end_access(shm_buffer);

        GLint currentFramebuffer { 0 };
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);
        resource().flags(currentFramebuffer == 0 ? ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT : 0);
    }
    else // DMA buffer
    {
        LDMABuffer *dmaBuffer { static_cast<LDMABuffer*>(wl_resource_get_user_data(resource().buffer())) };
        UInt32 i { 0 };
        EGLAttrib attribs[50];
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

        if (dmaBuffer->planes()->num_fds > 1)
        {
            attribs[i++] = EGL_DMA_BUF_PLANE1_FD_EXT;
            attribs[i++] = dmaBuffer->planes()->fds[1];
            attribs[i++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
            attribs[i++] = dmaBuffer->planes()->offsets[1];
            attribs[i++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
            attribs[i++] = dmaBuffer->planes()->strides[1];
            attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
            attribs[i++] = dmaBuffer->planes()->modifiers[1] & 0xFFFFFFFF;
            attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
            attribs[i++] = dmaBuffer->planes()->modifiers[1] >> 32;

            if (dmaBuffer->planes()->num_fds > 2)
            {
                attribs[i++] = EGL_DMA_BUF_PLANE2_FD_EXT;
                attribs[i++] = dmaBuffer->planes()->fds[2];
                attribs[i++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
                attribs[i++] = dmaBuffer->planes()->offsets[2];
                attribs[i++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
                attribs[i++] = dmaBuffer->planes()->strides[2];
                attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
                attribs[i++] = dmaBuffer->planes()->modifiers[2] & 0xFFFFFFFF;
                attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
                attribs[i++] = dmaBuffer->planes()->modifiers[2] >> 32;

                if (dmaBuffer->planes()->num_fds > 3)
                {
                    attribs[i++] = EGL_DMA_BUF_PLANE3_FD_EXT;
                    attribs[i++] = dmaBuffer->planes()->fds[3];
                    attribs[i++] = EGL_DMA_BUF_PLANE3_OFFSET_EXT;
                    attribs[i++] = dmaBuffer->planes()->offsets[3];
                    attribs[i++] = EGL_DMA_BUF_PLANE3_PITCH_EXT;
                    attribs[i++] = dmaBuffer->planes()->strides[3];
                    attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT;
                    attribs[i++] = dmaBuffer->planes()->modifiers[3] & 0xFFFFFFFF;
                    attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT;
                    attribs[i++] = dmaBuffer->planes()->modifiers[3] >> 32;
                }
            }
        }

        attribs[i++] = EGL_IMAGE_PRESERVED_KHR;
        attribs[i++] = EGL_TRUE;
        attribs[i++] = EGL_NONE;

        EGLImage image { eglCreateImage(compositor()->eglDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs) };

        if (image == EGL_NO_IMAGE)
        {
            resource().failed();
            return -1;
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
            return -1;
        }

        if (resource().screenCopyManagerRes())
        {
            auto &outputDamage { resource().screenCopyManagerRes()->damage[resource().output()] };
            outputDamage.damage.op(resource().rectB(), SkRegion::Op::kIntersect_Op);

            // No damage, wait...
            if (resource().waitForDamage() && outputDamage.damage.isEmpty())
            {
                glDeleteFramebuffers(1, &fb);
                glDeleteRenderbuffers(1, &rb);
                eglDestroyImage(compositor()->eglDisplay(), image);
                return 0;
            }

            damage = std::move(outputDamage.damage);
        }
        else
        {
            // No damage tracking
            damage.op(resource().rectB(), SkRegion::kUnion_Op);
        }

        LFramebufferWrapper glFb(
            fb,
            SkISize((Int32)dmaBuffer->planes()->width, (Int32)dmaBuffer->planes()->height),
            resource().rectB().topLeft());

        LTexture *outputTexture { resource().output()->bufferTexture(resource().output()->currentBuffer()) };
        LPainter &p { *resource().output()->painter() };
        LFramebuffer *prevFb { p.boundFramebuffer() };
        /* TODO: glFinish();*/
        p.setAlpha(1.f);
        p.setColorFactor(1.f, 1.f, 1.f, 1.f);
        p.bindFramebuffer(&glFb);
        p.enableCustomTextureColor(false);
        p.bindTextureMode({
            .texture = outputTexture,
            .pos = SkIPoint(0, 0),
            .srcRect = SkRect::MakeWH(outputTexture->sizeB().width(), outputTexture->sizeB().height()),
            .dstSize = glFb.rect().size(),
            .srcTransform = CZTransform::Normal,
            .srcScale = 1.f,
        });
        glDisable(GL_BLEND);
        p.drawRect(resource().rectB());
        /* TODO: glFinish(); */
        p.bindFramebuffer(prevFb);
        glDeleteFramebuffers(1, &fb);
        glDeleteRenderbuffers(1, &rb);
        eglDestroyImage(compositor()->eglDisplay(), image);
        resource().flags(0);
    }

    if (resource().waitForDamage())
    {
        damage.translate(-resource().rectB().x(), -resource().rectB().y());
        resource().damage(damage);
    }

    /* Backend presentation time may not be available, use LTime instead */
    resource().ready(LTime::ns());
    return 1;
}
