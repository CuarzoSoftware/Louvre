#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LRect.h>

#include <stdio.h>
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

LTexture::LTexture(LCompositor *compositor, GLuint textureUnit)
{
    m_imp = new LTexturePrivate();
    imp()->compositor = compositor;
    imp()->unit = textureUnit;
}

LCompositor *LTexture::compositor() const
{
    return imp()->compositor;
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

bool LTexture::updateRect(const LRect &rect, UInt32 stride, const void *buffer)
{
    if (initialized())
        return compositor()->imp()->graphicBackend->updateTextureRect(this, stride, rect, buffer);

    return false;
}

LTexture::~LTexture()
{
    imp()->deleteTexture(this);
    delete m_imp;
}

void LTexture::LTexturePrivate::deleteTexture(LTexture *texture)
{
    glActiveTexture(GL_TEXTURE0 + unit);

    if(graphicBackendData)
    {
        compositor->imp()->graphicBackend->destroyTexture(texture);
        graphicBackendData = nullptr;
    }
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

LTexture::LTexturePrivate *LTexture::imp() const
{
    return m_imp;
}

