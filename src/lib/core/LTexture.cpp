#include <private/LPainterPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LRenderBufferPrivate.h>
#include <LTextureView.h>
#include <FreeImage.h>
#include <LRect.h>
#include <LLog.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace Louvre;
using namespace std;

UInt32 LTexture::waylandFormatToDRM(UInt32 waylandFormat)
{
    if (waylandFormat == WL_SHM_FORMAT_ARGB8888)
        return DRM_FORMAT_ARGB8888;
    else if (waylandFormat == WL_SHM_FORMAT_XRGB8888)
        return DRM_FORMAT_XRGB8888;

    return waylandFormat;
}

UInt32 LTexture::formatBytesPerPixel(UInt32 format)
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

UInt32 LTexture::formatPlanes(UInt32 format)
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
    case DRM_FORMAT_P030:
        return 2;
        break;
    default:
        return 1;
    }
}

LTexture::LTexture()
{
    m_imp = new LTexturePrivate();
    imp()->texture = this;
    compositor()->imp()->textures.push_back(this);
    imp()->compositorLink = std::prev(compositor()->imp()->textures.end());
}

bool LTexture::setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer)
{
    if (imp()->sourceType == Framebuffer)
        return false;

    imp()->deleteTexture();

    if (compositor()->imp()->graphicBackend->createTextureFromCPUBuffer(this, size, stride, format, buffer))
    {
        imp()->format = format;
        imp()->sizeB = size;
        imp()->sourceType = CPU;
        return true;
    }

    return false;
}

bool LTexture::setData(void *wlDRMBuffer)
{
    if (imp()->sourceType == Framebuffer)
        return false;

    imp()->deleteTexture();

    if (compositor()->imp()->graphicBackend->createTextureFromWaylandDRM(this, wlDRMBuffer))
    {
        imp()->sourceType = WL_DRM;
        return true;
    }

    return false;
}

bool LTexture::setDataB(const LDMAPlanes *planes)
{
    if (imp()->sourceType == Framebuffer)
        return false;

    imp()->deleteTexture();

    if (compositor()->imp()->graphicBackend->createTextureFromDMA(this, planes))
    {
        imp()->sourceType = DMA;
        return true;
    }

    return false;
}

bool LTexture::updateRect(const LRect &rect, UInt32 stride, const void *buffer)
{

    if (initialized() && imp()->sourceType != Framebuffer)
    {
        imp()->increaseSerial();
        return compositor()->imp()->graphicBackend->updateTextureRect(this, stride, rect, buffer);
    }

    return false;
}

Louvre::LTexture *LTexture::copyB(const LSize &dst, const LRect &src, bool highQualityScaling) const
{
    if (!initialized())
        return nullptr;

    if (dst.w() < 0 || dst.h() < 0)
    {
        LLog::error("Failed to copy texture. Invalid destination size.");
        return nullptr;
    }

    LPainter *painter = nullptr;
    std::thread::id threadId = std::this_thread::get_id();

    if (threadId == LCompositor::compositor()->mainThreadId())
        painter = LCompositor::compositor()->imp()->painter;
    else
    {
        for (LOutput *o : LCompositor::compositor()->outputs())
        {
            if (o->state() == LOutput::Initialized && o->imp()->threadId == threadId)
            {
                painter = o->painter();
                break;
            }
        }
    }

    if (!painter)
    {
        LLog::error("Failed to copy texture. No painter found.");
        return nullptr;
    }

    GLuint textureId = id(painter->imp()->output);

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
        LLog::error("Failed to copy texture. Invalid size.");
        return nullptr;
    }

    LTexture *textureCopy;
    bool ret = false;

    if (highQualityScaling)
    {
        Int32 wScale = round(abs(Float64(srcRect.w()) / Float64(dstSize.w())));
        Int32 hScale = round(abs(Float64(srcRect.h()) / Float64(dstSize.h())));

        // Check if downscaling is needed
        if (wScale <= 2 && hScale <= 2)
            goto skipHQ;

        Float64 pixSizeW = 1.0 / Float64(sizeB().w());
        Float64 pixSizeH = 1.0 / Float64(sizeB().h());

        Float64 limit = 3.f;

        if (wScale > limit)
        {
            pixSizeW *= Float64(wScale) / limit;
            wScale = limit;
        }

        if (hScale > limit)
        {
            pixSizeH *= Float64(hScale) / limit;
            hScale = limit;
        }

        Float64 colorFactor = 1.0 / Float64(wScale*hScale);

        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        GLuint texCopy;
        glGenTextures(1, &texCopy);
        glBindTexture(GL_TEXTURE_2D, texCopy);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstSize.w(), dstSize.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texCopy, 0);

        GLenum tgt = target();
        painter->imp()->switchTarget(tgt);
        glEnable(GL_BLEND);
        glScissor(0, 0, dstSize.w(), dstSize.h());
        glViewport(0, 0, dstSize.w(), dstSize.h());
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        painter->imp()->shaderSetAlpha(1.f);
        painter->imp()->shaderSetMode(3);
        painter->imp()->shaderSetActiveTexture(0);
        painter->imp()->shaderSetTexSize(sizeB().w(), sizeB().h());
        painter->imp()->shaderSetSrcRect(srcRect.x(), srcRect.y() + srcRect.h(), srcRect.w(), -srcRect.h());

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

        painter->imp()->shaderSetSamplerBounds(x1, y1, x2, y2);
        painter->imp()->shaderSetColorFactor(colorFactor, colorFactor, colorFactor, colorFactor);

        glBindTexture(tgt, id(painter->imp()->output));
        glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBlendFunc(GL_ONE, GL_ONE);

        for (Int32 x = 0; x < wScale; x++)
        {
            for (Int32 y = 0; y < hScale; y++)
            {
                painter->imp()->shaderSetSamplerOffset(x * pixSizeW, y * pixSizeH);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
        }

        glFinish();
        painter->imp()->shaderSetColorFactor(1.f, 1.f, 1.f, 1.f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        textureCopy = new LTexture();

        if (compositor()->imp()->graphicBackend->rendererGPUs() == 1)
        {
            ret = textureCopy->imp()->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_BGRA8888, dstSize, painter->imp()->output);
        }
        else
        {
            UChar8 *cpuBuffer = (UChar8*)malloc(dstSize.w() * dstSize.h() * 4);

            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glPixelStorei(GL_PACK_ROW_LENGTH, dstSize.w());
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            glReadPixels(0, 0, dstSize.w(), dstSize.h(), GL_BGRA, GL_UNSIGNED_BYTE, cpuBuffer);
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);

            textureCopy = new LTexture();
            ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);

            free(cpuBuffer);
            glDeleteTextures(1, &texCopy);
        }

        glDeleteFramebuffers(1, &framebuffer);

        if (ret)
            return textureCopy;

        LLog::error("Failed to create texture. Graphical backend error.");
        delete textureCopy;
        return nullptr;
    }

    skipHQ:

    // If single GPU, no need for DMA sharing
    if (compositor()->imp()->graphicBackend->rendererGPUs() == 1)
    {
        // Direct copy glCopyTexImage2D()
        if (target() == GL_TEXTURE_2D &&
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
            glBindTexture(GL_TEXTURE_2D, texCopy);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcRect.x(), srcRect.y(), srcRect.w(), srcRect.h(), 0);
            glFinish();
            textureCopy = new LTexture();
            ret = textureCopy->imp()->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_BGRA8888, dstSize, painter->imp()->output);
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
            glBindTexture(GL_TEXTURE_2D, texCopy);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstSize.w(), dstSize.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texCopy, 0);
            painter->imp()->scaleTexture((LTexture*)this, srcRect, dstSize);
            glFinish();
            textureCopy = new LTexture();
            ret = textureCopy->imp()->setDataB(texCopy, GL_TEXTURE_2D, DRM_FORMAT_BGRA8888, dstSize, painter->imp()->output);
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
            LLog::error("Failed to copy texture. Could not create framebuffer.");
            return nullptr;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);

        if (renderbuffer == 0)
        {
            LLog::error("Failed to copy texture. Could not create renderbuffer.");
            glDeleteFramebuffers(1, &framebuffer);
            return nullptr;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, dstSize.w(), dstSize.h());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LLog::error("Failed to copy texture. Incomplete framebuffer.");
            glDeleteRenderbuffers(1, &renderbuffer);
            glDeleteFramebuffers(1, &framebuffer);
            return nullptr;
        }

        UChar8 *cpuBuffer = (UChar8*)malloc(dstSize.w() * dstSize.h() * 4);

        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, dstSize.w());
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glReadPixels(0, 0, dstSize.w(), dstSize.h(), GL_BGRA, GL_UNSIGNED_BYTE, cpuBuffer);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);

        textureCopy = new LTexture();
        ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);

        free(cpuBuffer);

        glDeleteRenderbuffers(1, &renderbuffer);
        glDeleteFramebuffers(1, &framebuffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ret)
        return textureCopy;

    LLog::error("Failed to create texture. Graphical backend error.");
    delete textureCopy;
    return nullptr;
}

bool LTexture::save(const char *path) const
{
    if (!initialized())
    {
        LLog::error("Failed to save texture. Uninitialized texture.");
        return false;
    }

    if (!path)
    {
        LLog::error("Failed to save texture. Invalid path.");
        return false;
    }

    if (sizeB().area() <= 0)
    {
        LLog::error("Failed to save texture. Invalid size.");
        return false;
    }

    LPainter *painter = nullptr;
    std::thread::id threadId = std::this_thread::get_id();

    if (threadId == LCompositor::compositor()->mainThreadId())
        painter = LCompositor::compositor()->imp()->painter;
    else
    {
        for (LOutput *o : LCompositor::compositor()->outputs())
        {
            if (o->state() == LOutput::Initialized && o->imp()->threadId == threadId)
            {
                painter = o->painter();
                break;
            }
        }
    }

    if (!painter)
    {
        LLog::error("Failed to save texture. No painter found.");
        return false;
    }

    {
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);

        if (!framebuffer)
            goto draw;

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        GLuint texId = id(painter->imp()->output);
        glBindTexture(GL_TEXTURE_2D, texId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glDeleteFramebuffers(1, &framebuffer);
            goto draw;
        }

        UChar8 *cpuBuffer = (UChar8 *)malloc(sizeB().w()*sizeB().h()*4);

        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, sizeB().w());
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glReadPixels(0, 0, sizeB().w(), sizeB().h(), GL_BGRA, GL_UNSIGNED_BYTE, cpuBuffer);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);

        FIBITMAP *image = FreeImage_ConvertFromRawBits(cpuBuffer,
                                                       sizeB().w(),
                                                       sizeB().h(),
                                                       4 * sizeB().w(),
                                                       32,
                                                       0xFF0000, 0x00FF00, 0x0000FF,
                                                       true);

        bool ret = FreeImage_Save(FIF_PNG, image, path, PNG_DEFAULT);

        FreeImage_Unload(image);
        free(cpuBuffer);
        glDeleteFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (ret)
            return true;

        LLog::error("Failed to copy texture. FreeImage error.");
        return false;
    }

    draw:

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);

    if (framebuffer == 0)
    {
        LLog::error("Failed to save texture. Could not create framebuffer.");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint renderbuffer;
    glGenRenderbuffers(1, &renderbuffer);

    if (renderbuffer == 0)
    {
        LLog::error("Failed to save texture. Could not create renderbuffer.");
        glDeleteFramebuffers(1, &framebuffer);
        return false;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, sizeB().w(), sizeB().h());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LLog::error("Failed to save texture. Incomplete framebuffer.");
        glDeleteRenderbuffers(1, &renderbuffer);
        glDeleteFramebuffers(1, &framebuffer);
        return false;
    }

    painter->imp()->scaleTexture((LTexture*)this, LSize(sizeB().w(), -sizeB().h()), sizeB());

    UChar8 *cpuBuffer = (UChar8 *)malloc(sizeB().w()*sizeB().h()*4);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, sizeB().w());
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glReadPixels(0, 0, sizeB().w(), sizeB().h(), GL_BGRA, GL_UNSIGNED_BYTE, cpuBuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);

    FIBITMAP *image = FreeImage_ConvertFromRawBits(cpuBuffer,
                                                   sizeB().w(),
                                                   sizeB().h(),
                                                   4 * sizeB().w(),
                                                   32,
                                                   0xFF0000, 0x00FF00, 0x0000FF,
                                                   false);

    bool ret = FreeImage_Save(FIF_PNG, image, path, PNG_DEFAULT);

    FreeImage_Unload(image);
    free(cpuBuffer);
    glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ret)
        return true;

    LLog::error("Failed to copy texture. FreeImage error.");
    return false;
}

LTexture::~LTexture()
{
    while (!imp()->textureViews.empty())
        imp()->textureViews.back()->setTexture(nullptr);

    imp()->deleteTexture();
    compositor()->imp()->textures.erase(imp()->compositorLink);
    delete m_imp;
}

void LTexture::LTexturePrivate::deleteTexture()
{
    if (texture == compositor()->cursor()->imp()->defaultTexture)
        compositor()->cursor()->replaceDefaultB(nullptr, 0);

    if (texture == compositor()->cursor()->texture())
        compositor()->cursor()->useDefault();

    increaseSerial();

    if (texture->sourceType() == Framebuffer)
    {
        return;
    }

    if (texture->sourceType() == Native)
    {
        if (nativeOutput)
            nativeOutput->imp()->nativeTexturesToDestroy.push_back(nativeId);
        else
            compositor()->imp()->nativeTexturesToDestroy.push_back(nativeId);

        graphicBackendData = nullptr;
        return;
    }

    if (graphicBackendData)
    {
        compositor()->imp()->graphicBackend->destroyTexture(texture);
        graphicBackendData = nullptr;
    }
}

void LTexture::LTexturePrivate::increaseSerial()
{
    serial++;
}

bool LTexture::LTexturePrivate::setDataB(GLuint textureId, GLenum target, UInt32 format, const LSize &size, LOutput *output)
{
    if (sourceType == Framebuffer)
        return false;

    deleteTexture();
    sourceType = Native;
    graphicBackendData = &nativeId;
    nativeId = textureId;
    nativeTarget = target;
    nativeOutput = output;
    this->format = format;
    sizeB = size;
    return true;
}

const LSize &LTexture::sizeB() const
{
    return imp()->sizeB;
}

bool LTexture::initialized() const
{
    return imp()->graphicBackendData != nullptr;
}

GLuint LTexture::id(LOutput *output) const
{
    if (initialized())
    {
        if (sourceType() == Framebuffer)
        {
            LRenderBuffer *fb = (LRenderBuffer*)imp()->graphicBackendData;
            return fb->imp()->getTextureId();
        }
        else if (sourceType() == Native)
        {
            return imp()->nativeId;
        }
        else
            return compositor()->imp()->graphicBackend->getTextureID(output, (LTexture*)this);
    }

    return 0;
}

GLenum LTexture::target() const
{
    if (initialized())
    {
        if (sourceType() == Framebuffer)
            return GL_TEXTURE_2D;

        else if (sourceType() == Native)
            return imp()->nativeTarget;

        return compositor()->imp()->graphicBackend->getTextureTarget((LTexture*)this);
    }

    return GL_TEXTURE_2D;
}

LTexture::BufferSourceType LTexture::sourceType() const
{
    return imp()->sourceType;
}

UInt32 LTexture::format() const
{
    return imp()->format;
}
