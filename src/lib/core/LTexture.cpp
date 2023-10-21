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
    compositor()->imp()->textures.push_back(this);
    imp()->compositorLink = std::prev(compositor()->imp()->textures.end());
}

bool LTexture::setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer)
{
    if (imp()->sourceType == Framebuffer)
        return false;

    imp()->deleteTexture(this);

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

    imp()->deleteTexture(this);

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

    imp()->deleteTexture(this);

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

    LRect srcRect;
    LSize dstSize;

    if (src == LRect())
        srcRect = LRect(0, sizeB());
    else
        srcRect = src;

    if (dst == LSize())
        dstSize = sizeB();
    else
        dstSize = dst;

    if (highQualityScaling)
    {
        // Check if downscaling is needed
        if (abs(srcRect.w() / dstSize.w()) <= 2.f && abs(srcRect.h() / dstSize.h()) <= 2.f)
            goto skipDownscaling;

        GLuint texFb, texId, rndFb, rndId;
        glGenFramebuffers(1, &texFb);
        glBindFramebuffer(GL_FRAMEBUFFER, texFb);
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeB().w(), sizeB().h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
        painter->imp()->scaleTexture(id(painter->imp()->output), target(), texFb, GL_LINEAR, sizeB(), LRect(0, sizeB()), sizeB());
        glBindTexture(GL_TEXTURE_2D, texId);
        glGenerateMipmap(GL_TEXTURE_2D);

        glGenFramebuffers(1, &rndFb);
        glBindFramebuffer(GL_FRAMEBUFFER, rndFb);
        glGenRenderbuffers(1, &rndId);
        glBindRenderbuffer(GL_RENDERBUFFER, rndId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, dstSize.w(), dstSize.h());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rndId);
        painter->imp()->scaleTexture(texId, GL_TEXTURE_2D, rndFb, GL_LINEAR_MIPMAP_LINEAR, sizeB(), srcRect, dstSize);

        glBindFramebuffer(GL_FRAMEBUFFER, rndFb);

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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        LTexture *textureCopy = new LTexture();
        bool ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);

        free(cpuBuffer);

        glDeleteTextures(1, &texId);
        glDeleteFramebuffers(1, &texFb);
        glDeleteRenderbuffers(1, &rndId);
        glDeleteFramebuffers(1, &rndFb);

        if (ret)
            return textureCopy;

        LLog::error("Failed to create texture. Graphical backend error.");
        delete textureCopy;
        return nullptr;
    }

    skipDownscaling:

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

    painter->imp()->scaleTexture((LTexture*)this, srcRect, dstSize);

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

    LTexture *textureCopy = new LTexture();
    bool ret = textureCopy->setDataB(dstSize, dstSize.w() * 4, DRM_FORMAT_ARGB8888, cpuBuffer);

    free(cpuBuffer);
    glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &framebuffer);
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

    imp()->deleteTexture(this);
    compositor()->imp()->textures.erase(imp()->compositorLink);
    delete m_imp;
}

void LTexture::LTexturePrivate::deleteTexture(LTexture *texture)
{
    if (texture == compositor()->cursor()->imp()->defaultTexture)
        compositor()->cursor()->replaceDefaultB(nullptr, 0);

    if (texture == compositor()->cursor()->texture())
        compositor()->cursor()->useDefault();

    increaseSerial();

    if (texture->sourceType() == Framebuffer)
        return;

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
        else
            return compositor()->imp()->graphicBackend->getTextureID(output, (LTexture*)this);
    }

    return 0;
}

GLenum LTexture::target() const
{
    if (initialized() && sourceType() != Framebuffer)
        return compositor()->imp()->graphicBackend->getTextureTarget((LTexture*)this);

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
