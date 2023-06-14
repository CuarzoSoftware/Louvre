#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
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

bool LTexture::initialized()
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
