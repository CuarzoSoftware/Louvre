#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <other/stb_image_write.h>
#include <private/LPainterPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <LTextureView.h>
#include <LRect.h>
#include <LLog.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace Louvre;
using namespace std;

LTexture::LTexture() noexcept
{
    compositor()->imp()->textures.push_back(this);
}

LTexture::~LTexture() noexcept
{
    reset();
    LVectorRemoveOneUnordered(compositor()->imp()->textures, this);
}

UInt32 LTexture::waylandFormatToDRM(UInt32 waylandFormat) noexcept
{
    if (waylandFormat == WL_SHM_FORMAT_ARGB8888)
        return DRM_FORMAT_ARGB8888;
    else if (waylandFormat == WL_SHM_FORMAT_XRGB8888)
        return DRM_FORMAT_XRGB8888;

    return waylandFormat;
}

UInt32 LTexture::formatBytesPerPixel(UInt32 format) noexcept
{
    switch (format)
    {
    case DRM_FORMAT_C8:
    case DRM_FORMAT_RGB332:
    case DRM_FORMAT_BGR233:
        return 1;
        break;
    case DRM_FORMAT_XRGB1555:
    case DRM_FORMAT_XBGR1555:
    case DRM_FORMAT_RGBX5551:
    case DRM_FORMAT_BGRX5551:
    case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_RGBA5551:
    case DRM_FORMAT_BGRA5551:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
        return 2;
        break;
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
        return 3;
        break;
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGBX8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_RGBX1010102:
    case DRM_FORMAT_BGRX1010102:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_RGBA1010102:
    case DRM_FORMAT_BGRA1010102:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
        return 4;
        break;
    default:
        return 0;
    }
}

UInt32 LTexture::formatPlanes(UInt32 format) noexcept
{
    switch (format)
    {
    case DRM_FORMAT_Q410:
    case DRM_FORMAT_Q401:
    case DRM_FORMAT_YUV410:
    case DRM_FORMAT_YVU410:
    case DRM_FORMAT_YUV411:
    case DRM_FORMAT_YVU411:
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_YVU422:
    case DRM_FORMAT_YUV444:
    case DRM_FORMAT_YVU444:
        return 3;
        break;
    case DRM_FORMAT_XRGB8888_A8:
    case DRM_FORMAT_XBGR8888_A8:
    case DRM_FORMAT_RGBX8888_A8:
    case DRM_FORMAT_BGRX8888_A8:
    case DRM_FORMAT_RGB888_A8:
    case DRM_FORMAT_BGR888_A8:
    case DRM_FORMAT_RGB565_A8:
    case DRM_FORMAT_BGR565_A8:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV16:
    case DRM_FORMAT_NV61:
    case DRM_FORMAT_NV24:
    case DRM_FORMAT_NV42:
    case DRM_FORMAT_NV15:
    case DRM_FORMAT_P210:
    case DRM_FORMAT_P010:
    case DRM_FORMAT_P012:
    case DRM_FORMAT_P016:
        return 2;
        break;
    default:
        return 1;
    }
}

bool LTexture::setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromCPUBuffer(this, size, stride, format, buffer))
    {
        m_format = format;
        m_sizeB = size;
        m_sourceType = CPU;
        return true;
    }

    return false;
}

bool LTexture::setData(void *wlDRMBuffer) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromWaylandDRM(this, wlDRMBuffer))
    {
        m_sourceType = WL_DRM;
        return true;
    }

    return false;
}

bool LTexture::setDataB(const LDMAPlanes *planes) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromDMA(this, planes))
    {
        m_sourceType = DMA;
        return true;
    }

    return false;
}

bool LTexture::updateRect(const LRect &rect, UInt32 stride, const void *buffer) noexcept
{

    if (initialized() && m_sourceType != Framebuffer)
    {
        m_serial++;
        return compositor()->imp()->graphicBackend->textureUpdateRect(this, stride, rect, buffer);
    }

    return false;
}

LTexture *LTexture::copyB(const LSize &dst, const LRect &src, bool highQualityScaling) const noexcept
{
    if (!initialized())
        return nullptr;

    if (dst.w() < 0 || dst.h() < 0)
    {
        LLog::error("[LTexture::copyB] Failed to copy texture. Invalid destination size.");
        return nullptr;
    }

    LPainter *painter = compositor()->imp()->findPainter();

    if (!painter)
    {
        LLog::error("[LTexture::copyB] Failed to copy texture. No painter found.");
        return nullptr;
    }

    LRect srcRect;
    LSize dstSize;

    if (src == LRect())
        srcRect = LRectF(0, sizeB());
    else
        srcRect = src;

    if (dst == LSize())
        dstSize = sizeB();
    else
        dstSize = dst;

    if (srcRect.w() == 0 || srcRect.h() == 0 || dstSize.w() == 0 || dstSize.h() == 0)
    {
        LLog::error("[LTexture::copyB] Failed to copy texture. Invalid size.");
        return nullptr;
    }

    GLuint textureId = id(painter->imp()->output);
    LTexture *textureCopy;
    bool ret = false;

    if (highQualityScaling)
    {
        Float32 wScaleF = fabs(Float32(srcRect.w()) / Float32(dstSize.w()));
        Float32 hScaleF = fabs(Float32(srcRect.h()) / Float32(dstSize.h()));

        // Check if HQ downscaling is needed
        if (wScaleF <= 2.f && hScaleF <= 2.f)
            goto skipHQ;

        GLenum textureTarget = target();
        GLuint prevProgram = painter->imp()->currentProgram;

        if (textureTarget == GL_TEXTURE_EXTERNAL_OES)
        {
            if (!painter->imp()->programObjectScalerExternal)
                goto skipHQ;

            glUseProgram(painter->imp()->programObjectScalerExternal);
            painter->imp()->currentUniformsScaler = &painter->imp()->uniformsScalerExternal;
        }
        else
        {
            glUseProgram(painter->imp()->programObjectScaler);
            painter->imp()->currentUniformsScaler = &painter->imp()->uniformsScaler;
        }

        Int32 wScale = ceilf(wScaleF);
        Int32 hScale = ceilf(hScaleF);

        Float32 limit = 10.f;

        if (wScale > limit)
            wScale = limit;

        if (hScale > limit)
            hScale = limit;

        Float32 pixSizeW = wScaleF / Float32(sizeB().w() * wScale);
        Float32 pixSizeH = hScaleF / Float32(sizeB().h() * hScale);

        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        GLuint texCopy;
        glGenTextures(1, &texCopy);
        LTexture::LTexturePrivate::setTextureParams(texCopy, GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstSize.w(), dstSize.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texCopy, 0);
        glDisable(GL_BLEND);
        glScissor(0, 0, dstSize.w(), dstSize.h());
        glViewport(0, 0, dstSize.w(), dstSize.h());
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(painter->imp()->currentUniformsScaler->activeTexture, 0);
        glUniform2f(painter->imp()->currentUniformsScaler->texSize, sizeB().w(), sizeB().h());
        glUniform4f(painter->imp()->currentUniformsScaler->srcRect, srcRect.x(), srcRect.y() + srcRect.h(), srcRect.w(), -srcRect.h());

        Float32 tmp;
        Float32 x1 = (Float32(srcRect.x()) + 0.f * Float32(srcRect.w())) / Float32(sizeB().w());
        Float32 y1 = (Float32(srcRect.y() + srcRect.h()) + Float32(-srcRect.h()) - 0.f * Float32(-srcRect.h())) / Float32(sizeB().h());
        Float32 x2 = (Float32(srcRect.x()) + 1.f * Float32(srcRect.w())) / Float32(sizeB().w());
        Float32 y2 = (Float32(srcRect.y() + srcRect.h()) + Float32(-srcRect.h()) - 1.f * Float32(-srcRect.h())) / Float32(sizeB().h());

        if (x1 > x2)
        {
            tmp = x1;
            x1 = x2;
            x2 = tmp;
        }

        if (y1 > y2)
        {
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        glUniform4f(painter->imp()->currentUniformsScaler->samplerBounds, x1, y1, x2, y2);
        LTexture::LTexturePrivate::setTextureParams(textureId, textureTarget, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
        glUniform2f(painter->imp()->currentUniformsScaler->pixelSize, pixSizeW, pixSizeH);
        glUniform2i(painter->imp()->currentUniformsScaler->iters, wScale, hScale);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        textureCopy = new LTexture();

        if (compositor()->imp()->graphicBackend->backendGetRendererGPUs() == 1)
        {
            glFlush();
            ret = textureCopy->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, painter->imp()->output);
        }
        else
        {
            UChar8 *cpuBuffer = (UChar8*)malloc(dstSize.w() * dstSize.h() * 4);
            GLenum glFormat = painter->imp()->openGLExtensions.EXT_read_format_bgra ? GL_BGRA_EXT : GL_RGBA;
            LTexture::LTexturePrivate::readPixels(LRect(0, dstSize), 0, dstSize.w(), glFormat, GL_UNSIGNED_BYTE, cpuBuffer);
            textureCopy = new LTexture();

            if (glFormat == GL_BGRA_EXT)
                ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);
            else
            {
                if (painter->imp()->cpuFormats.ABGR8888)
                    ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ABGR8888, cpuBuffer);
                else
                {
                    UChar8 tmp;

                    for (Int32 i = 0; i < dstSize.area() * 4; i+=4)
                    {
                        tmp = cpuBuffer[i];
                        cpuBuffer[i] = cpuBuffer[i+2];
                        cpuBuffer[i+2] = tmp;
                    }

                    ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);
                }
            }
            free(cpuBuffer);
            glDeleteTextures(1, &texCopy);
        }

        glDeleteFramebuffers(1, &framebuffer);
        glUseProgram(prevProgram);

        if (ret)
            return textureCopy;

        LLog::error("[LTexture::copyB] Failed to create texture. Graphical backend error.");
        delete textureCopy;
        return nullptr;
    }

    skipHQ:

    // If single GPU, no need for DMA sharing
    if (compositor()->imp()->graphicBackend->backendGetRendererGPUs() == 1)
    {
        GLenum textureTarget = target();

        // Direct copy glCopyTexImage2D()
        if (textureTarget == GL_TEXTURE_2D &&
            dstSize.w() == srcRect.w() &&
            dstSize.h() == srcRect.h() &&
            srcRect.x() >= 0 &&
            srcRect.x() + srcRect.w() <= sizeB().w() &&
            srcRect.y() >= 0 &&
            srcRect.y() + srcRect.h() <= sizeB().h())
        {
            GLuint framebuffer;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

            GLuint texCopy;
            glGenTextures(1, &texCopy);
            LTexture::LTexturePrivate::setTextureParams(texCopy, GL_TEXTURE_2D,
                                    GL_REPEAT, GL_REPEAT,
                                    GL_LINEAR, GL_LINEAR);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcRect.x(), srcRect.y(), srcRect.w(), srcRect.h(), 0);
            glFlush();
            textureCopy = new LTexture();
            ret = textureCopy->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, painter->imp()->output);
            glDeleteFramebuffers(1, &framebuffer);
        }
        // Scaled draw to new texture fb
        else
        {
            GLuint framebuffer;
            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            GLuint texCopy;
            glGenTextures(1, &texCopy);
            LTexture::LTexturePrivate::setTextureParams(texCopy, GL_TEXTURE_2D,
                                    GL_REPEAT, GL_REPEAT,
                                    GL_LINEAR, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstSize.w(), dstSize.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texCopy, 0);
            painter->imp()->scaleTexture((LTexture*)this, srcRect, dstSize);
            glFlush();
            textureCopy = new LTexture();
            ret = textureCopy->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, painter->imp()->output);
            glDeleteFramebuffers(1, &framebuffer);
        }
    }
    // If multi GPU, create DMA buffer
    else
    {
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);

        if (framebuffer == 0)
        {
            LLog::error("[LTexture::copyB] Failed to copy texture. Could not create framebuffer.");
            return nullptr;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);

        if (renderbuffer == 0)
        {
            LLog::error("[LTexture::copyB] Failed to copy texture. Could not create renderbuffer.");
            glDeleteFramebuffers(1, &framebuffer);
            return nullptr;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, dstSize.w(), dstSize.h());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LLog::error("[LTexture::copyB] Failed to copy texture. Incomplete framebuffer.");
            glDeleteRenderbuffers(1, &renderbuffer);
            glDeleteFramebuffers(1, &framebuffer);
            return nullptr;
        }

        painter->imp()->scaleTexture((LTexture*)this, srcRect, dstSize);

        UChar8 *cpuBuffer = (UChar8*)malloc(dstSize.w() * dstSize.h() * 4);
        GLenum glFormat = painter->imp()->openGLExtensions.EXT_read_format_bgra ? GL_BGRA_EXT : GL_RGBA;
        LTexture::LTexturePrivate::readPixels(LRect(0, dstSize), 0, dstSize.w(), glFormat, GL_UNSIGNED_BYTE, cpuBuffer);
        textureCopy = new LTexture();

        if (glFormat == GL_BGRA_EXT)
            ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);
        else
        {
            if (painter->imp()->cpuFormats.ABGR8888)
                ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ABGR8888, cpuBuffer);
            else
            {
                UChar8 tmp;

                for (Int32 i = 0; i < dstSize.area() * 4; i+=4)
                {
                    tmp = cpuBuffer[i];
                    cpuBuffer[i] = cpuBuffer[i+2];
                    cpuBuffer[i+2] = tmp;
                }

                ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);
            }
        }

        free(cpuBuffer);
        glDeleteRenderbuffers(1, &renderbuffer);
        glDeleteFramebuffers(1, &framebuffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ret)
        return textureCopy;

    LLog::error("[LTexture::copyB] Failed to create texture. Graphical backend error.");
    delete textureCopy;
    return nullptr;
}

bool LTexture::save(const std::filesystem::path &name) const noexcept
{
    if (name.empty())
    {
        LLog::error("[LTexture::save] Failed to save texture. Invalid path.");
        return false;
    }

    const char *error;
    LPainter *painter;
    GLuint framebuffer;
    UChar8 *buffer;

    if (!initialized())
    {
        error = "Uninitialized texture";
        goto printError;
    }

    if (sizeB().area() <= 0)
    {
        error = "Invalid size";
        goto printError;
    }

    painter = compositor()->imp()->findPainter();

    if (!painter)
    {
        error = "No LPainter found";
        goto printError;
    }

    glGenFramebuffers(1, &framebuffer);

    if (!framebuffer)
    {
        error = "Could not create framebuffer";
        goto printError;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    /* First attempt to read directly from the texture using a framebuffer. */
    {
        const GLuint textureId { id(painter->imp()->output) };
        const GLenum textureTarget { target() };

        glBindTexture(textureTarget, textureId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTarget, textureId, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LLog::warning("[LTexture::save] Failed to read texture directly using a framebuffer. Trying drawing the texture instead.");
            goto draw;
        }

        buffer = (UChar8 *)malloc(sizeB().w()*sizeB().h()*4);
        LTexture::LTexturePrivate::readPixels(LRect(0, sizeB()),
                          0, sizeB().w(),
                          GL_RGBA,
                          GL_UNSIGNED_BYTE, buffer);
        glDeleteFramebuffers(1, &framebuffer);

        goto save;
    }

    draw:

    /* If first attempt fails, then render texture into a render buffer and then to read the framebuffer. */
    {
        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);

        if (renderbuffer == 0)
        {
            glDeleteFramebuffers(1, &framebuffer);
            error = "Could not create renderbuffer";
            goto printError;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, sizeB().w(), sizeB().h());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteRenderbuffers(1, &renderbuffer);
            glDeleteFramebuffers(1, &framebuffer);
            error = "Incomplete framebuffer";
            goto printError;
        }

        painter->imp()->scaleTexture((LTexture*)this, LSize(sizeB().w(), -sizeB().h()), sizeB());
        buffer = (UChar8 *)malloc(sizeB().w()*sizeB().h()*4);
        LTexture::LTexturePrivate::readPixels(LRect(0, sizeB()),
                          0, sizeB().w(),
                          GL_RGBA,
                          GL_UNSIGNED_BYTE, buffer);
        glDeleteRenderbuffers(1, &renderbuffer);
        glDeleteFramebuffers(1, &framebuffer);
    }

    save:

    {
        const Int32 ret { stbi_write_png(name.c_str(), sizeB().w(), sizeB().h(), 4, buffer, sizeB().w() * 4) };
        free(buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (ret)
        {
            LLog::debug("[LTexture::save] Texture saved successfully: %s.", name.c_str());
            return true;
        }

        error = "STB Image error";
        goto printError;
    }

    printError:
    LLog::error("[LTexture::save] Failed to save texture: %s. %s.", name.c_str(), error);
    return false;
}

GLuint LTexture::id(LOutput *output) const noexcept
{
    if (initialized())
    {
        if (sourceType() == Framebuffer)
            return m_nativeId;
        else if (sourceType() == Native)
            return m_nativeId;
        else
            return compositor()->imp()->graphicBackend->textureGetID(output, (LTexture*)this);
    }

    return 0;
}

GLenum LTexture::backendTarget() const noexcept
{
    return compositor()->imp()->graphicBackend->textureGetTarget((LTexture*)this);
}

void LTexture::reset() noexcept
{
    if (cursor())
    {
        if (this == cursor()->imp()->defaultTexture)
            compositor()->cursor()->replaceDefaultB(nullptr, 0);
        if (this == cursor()->texture())
            cursor()->imp()->texture = cursor()->imp()->defaultTexture;
    }

    m_serial++;

    if (sourceType() == Framebuffer)
    {
        if (m_nativeId)
        {
            LLog::debug("[LTexture::reset] Native texture %d deleted.", m_nativeId);
            glDeleteTextures(1, &m_nativeId);
            m_nativeId = 0;
        }
        return;
    }

    if (sourceType() == Native)
    {
        if (m_nativeOutput.get())
            m_nativeOutput.get()->imp()->nativeTexturesToDestroy.push_back(m_nativeId);
        else
            compositor()->imp()->nativeTexturesToDestroy.push_back(m_nativeId);

        m_graphicBackendData = nullptr;
        return;
    }

    if (m_graphicBackendData)
    {
        compositor()->imp()->graphicBackend->textureDestroy(this);
        m_graphicBackendData = nullptr;
    }
}

bool LTexture::setDataB(GLuint textureId, GLenum target, UInt32 format, const LSize &size, LOutput *output) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();
    m_sourceType = Native;
    m_graphicBackendData = &m_nativeId;
    m_nativeId = textureId;
    m_nativeTarget = target;
    m_nativeOutput.reset(output);
    m_format = format;
    m_sizeB = size;
    return true;
}
