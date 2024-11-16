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

#if LOUVRE_USE_SKIA == 1
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/core/SkColorSpace.h>
#endif

using namespace Louvre;
using namespace std;

LTexture::LTexture(bool premultipliedAlpha) noexcept : m_premultipliedAlpha(premultipliedAlpha)
{
    compositor()->imp()->textures.push_back(this);
}

LTexture::~LTexture() noexcept
{
    notifyDestruction();
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

const std::vector<LDMAFormat> &LTexture::supportedDMAFormats() noexcept
{
    return *compositor()->imp()->graphicBackend->backendGetDMAFormats();
}

bool LTexture::setDataFromMainMemory(const LSize &size, UInt32 stride, UInt32 format, const void *buffer) noexcept
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

bool LTexture::setDataFromWaylandDRM(wl_resource *buffer) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromWaylandDRM(this, buffer))
    {
        m_sourceType = WL_DRM;
        return true;
    }

    return false;
}

bool LTexture::setDataFromDMA(const LDMAPlanes &planes) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromDMA(this, &planes))
    {
        m_sourceType = DMA;
        return true;
    }

    return false;
}

bool LTexture::setDataFromGL(GLuint id, GLenum target, UInt32 format, const LSize &size, bool transferOwnership) noexcept
{
    if (m_sourceType == Framebuffer)
        return false;

    reset();

    if (compositor()->imp()->graphicBackend->textureCreateFromGL(this, id, target, format, size, transferOwnership))
    {
        m_sourceType = GL;
        m_format = format;
        m_sizeB = size;
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

LTexture *LTexture::copy(const LSize &dst, const LRect &src, bool highQualityScaling) const noexcept
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

    if (true || src == LRect())
        srcRect = LRectF(0, sizeB());
    else
        srcRect = src;

    if (true || dst == LSize())
        dstSize = sizeB();
    else
        dstSize = dst;

    if (srcRect.w() == 0 || srcRect.h() == 0 || dstSize.w() == 0 || dstSize.h() == 0)
    {
        LLog::error("[LTexture::copyB] Failed to copy texture. Invalid size.");
        return nullptr;
    }

    GLuint textureId { id(painter->imp()->output) };
    LTexture *textureCopy { nullptr };
    bool ret = false;

// TODO
#if LOUVRE_USE_SKIA == 0
    if (highQualityScaling)
    {
        Float32 wScaleF = fabs(Float32(srcRect.w()) / Float32(dstSize.w()));
        Float32 hScaleF = fabs(Float32(srcRect.h()) / Float32(dstSize.h()));

        // Scale <= 2. Skipping HQ scaling as it's not required
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

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteTextures(1, &texCopy);
            glDeleteFramebuffers(1, &framebuffer);
            glUseProgram(prevProgram);
            LLog::error("[LTexture::copyB] glCheckFramebufferStatus failed. Skipping highQualityScaling.");
            goto skipHQ;
        }

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
        painter->imp()->shaderSetMode(LPainter::LPainterPrivate::LegacyMode);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        textureCopy = new LTexture(premultipliedAlpha());
        ret = textureCopy->setDataFromGL(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, true);
        glDeleteFramebuffers(1, &framebuffer);
        glUseProgram(prevProgram);

        if (ret)
        {
            // New texture copy (highQualityScaling = true)
            textureCopy->setFence();
            return textureCopy;
        }

        LLog::error("[LTexture::copyB] Failed to create texture. Graphic backend error.");
        delete textureCopy;
        return nullptr;
    }    
    skipHQ:
#else
    L_UNUSED(highQualityScaling)
#endif

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

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                glDeleteFramebuffers(1, &framebuffer);
                LLog::error("[LTexture::copyB] glCheckFramebufferStatus failed. Skipping glCopyTexImage2D method.");
                goto skipAll;
            }

            GLuint texCopy;
            glGenTextures(1, &texCopy);
            LTexture::LTexturePrivate::setTextureParams(texCopy, GL_TEXTURE_2D,
                                    GL_REPEAT, GL_REPEAT,
                                    GL_LINEAR, GL_LINEAR);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcRect.x(), srcRect.y(), srcRect.w(), srcRect.h(), 0);
            textureCopy = new LTexture(premultipliedAlpha());
            ret = textureCopy->setDataFromGL(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, true);
            glDeleteFramebuffers(1, &framebuffer);
        }
        // Scaled draw to new texture fb
        else
        {
            LFramebuffer *prevFb { painter->boundFramebuffer() };
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

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                glDeleteTextures(1, &texCopy);
                glDeleteFramebuffers(1, &framebuffer);
                LLog::error("[LTexture::copyB] glCheckFramebufferStatus failed. Skipping lowQualityScaling method.");
                goto skipAll;
            }

            LFramebufferWrapper wrapperFb(framebuffer, dstSize);
            painter->bindFramebuffer(&wrapperFb);
            painter->enableCustomTextureColor(false);
            painter->enableAutoBlendFunc(true);
            painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
            painter->setAlpha(1.f);
            painter->bindTextureMode({
                .texture = (LTexture*)this,
                .pos = LPoint(0,0),
                .srcRect = srcRect,
                .dstSize = dstSize,
                .srcTransform = LTransform::Normal,
                .srcScale = 1.f
            });
            glDisable(GL_BLEND);
            painter->drawRect(LRect(0, dstSize));
            glEnable(GL_BLEND);
            textureCopy = new LTexture(premultipliedAlpha());
            ret = textureCopy->setDataFromGL(texCopy, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, dstSize, true);
            glDeleteFramebuffers(1, &framebuffer);
            painter->bindFramebuffer(prevFb);
            // New texture copy (highQualityScaling = false)
        }
    }

skipAll:

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ret)
    {
        textureCopy->setFence();
        return textureCopy;
    }

    LLog::error("[LTexture::copyB] Failed to create texture. Graphical backend error.");
    if (textureCopy)
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
        LRenderBuffer rb { sizeB() };
        LFramebuffer *prevFb { painter->boundFramebuffer() };

        painter->bindFramebuffer(&rb);
        painter->enableCustomTextureColor(false);
        painter->setAlpha(1.f);
        painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
        painter->bindTextureMode(
        {
            .texture = (LTexture*)this,
            .pos = LPoint(0, 0),
            .srcRect = LRect(0, sizeB()),
            .dstSize = sizeB(),
            .srcTransform = LTransform::Normal,
            .srcScale = 1.f
        });
        glDisable(GL_BLEND);
        painter->drawRect(LRect(0, sizeB()));
        glEnable(GL_BLEND);
        buffer = (UChar8 *)malloc(sizeB().w()*sizeB().h()*4);
        LTexture::LTexturePrivate::readPixels(LRect(0, sizeB()),
                          0, sizeB().w(),
                          GL_RGBA,
                          GL_UNSIGNED_BYTE, buffer);

        painter->bindFramebuffer(prevFb);
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

void LTexture::setFence() noexcept
{
    if (initialized() && (m_sourceType == GL || m_sourceType == Framebuffer))
        compositor()->imp()->graphicBackend->textureSetFence(this);
}

GLuint LTexture::id(LOutput *output) const noexcept
{
    if (initialized())
        return compositor()->imp()->graphicBackend->textureGetID(output, (LTexture*)this);

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

    if (m_graphicBackendData)
    {
        compositor()->imp()->graphicBackend->textureDestroy(this);
        m_graphicBackendData = nullptr;
    }
}

#if LOUVRE_USE_SKIA == 1
const sk_sp<SkImage> LTexture::skImage() const noexcept
{
    LPainter *p { compositor()->imp()->findPainter() };

    if (!p || !p->skContext())
        return nullptr;

    if (m_skImage && m_skSerial == m_serial)
        return m_skImage;

    m_skSerial = m_serial;
    GrGLTextureInfo skTextureInfo;
    GrBackendTexture skTexture;
    skTextureInfo.fFormat = GL_BGRA8_EXT;
    skTextureInfo.fID = id(p->imp()->output);
    skTextureInfo.fTarget = target();

    skTexture = GrBackendTexture(
        sizeB().w(),
        sizeB().h(),
        GrMipMapped::kNo,
        skTextureInfo);

    static sk_sp<SkColorSpace> skColorSpace = SkColorSpace::MakeSRGB();

    m_skImage = SkImages::BorrowTextureFrom(
        p->skContext(),
        skTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        kBGRA_8888_SkColorType,
        SkAlphaType::kPremul_SkAlphaType,
        skColorSpace,
        nullptr,
        nullptr);

    return m_skImage;
}
#endif
