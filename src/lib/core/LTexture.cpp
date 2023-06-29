#include "LLog.h"
#include <private/LPainterPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <LRect.h>

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

LTexture::LTexture(GLuint textureUnit)
{
    m_imp = new LTexturePrivate();
    imp()->unit = textureUnit;
}

bool LTexture::setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer)
{
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
    if (initialized())
    {
        imp()->increaseSerial();
        return compositor()->imp()->graphicBackend->updateTextureRect(this, stride, rect, buffer);
    }

    return false;
}

Louvre::LTexture *LTexture::copyB(const LSize &dst, const LRect &src) const
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

    UChar8 *cpuBuffer = (UChar8 *)malloc(dstSize.w()*dstSize.h()*4);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, dstSize.w());
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glReadPixels(0, 0, dstSize.w(), dstSize.h(), GL_RGBA, GL_UNSIGNED_BYTE, cpuBuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);

    LTexture *textureCopy = new LTexture();
    bool ret = textureCopy->setDataB(dstSize, dstSize.w()*4, DRM_FORMAT_ABGR8888, cpuBuffer);

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

LTexture::~LTexture()
{
    imp()->deleteTexture(this);
    delete m_imp;
}

void LTexture::LTexturePrivate::deleteTexture(LTexture *texture)
{
    if (texture == compositor()->cursor()->texture())
        compositor()->cursor()->useDefault();

    increaseSerial();
    glActiveTexture(GL_TEXTURE0 + unit);

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

GLuint LTexture::id(LOutput *output)
{
    if (initialized())
        return compositor()->imp()->graphicBackend->getTextureID(output, this);

    return 0;
}

GLuint LTexture::unit()
{
    return imp()->unit;
}

LTexture::BufferSourceType LTexture::sourceType() const
{
    return imp()->sourceType;
}

GLenum LTexture::format() const
{
    return imp()->format;
}
