#include <private/LTexturePrivate.h>

#include <LRect.h>

#include <stdio.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace Louvre;
using namespace std;

static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = NULL;

LTexture::LTexture(GLuint textureUnit)
{
    m_imp = new LTexturePrivate();
    imp()->unit = textureUnit;
    glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress ("glEGLImageTargetTexture2DOES");
}

LTexture::~LTexture()
{
    imp()->deleteTexture();
    delete m_imp;
}

void LTexture::setDataB(Int32 width, Int32 height, void *data, GLenum format, GLenum type, Louvre::LTexture::BufferSourceType sourceType)
{
    imp()->deleteTexture();

    imp()->format = format;

    GLuint newTexture = 0;

    imp()->sizeB.setW(width);
    imp()->sizeB.setH(height);

    glGenTextures(1, &newTexture);
    glBindTexture (GL_TEXTURE_2D, newTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    imp()->id = newTexture;

    // EGL
    if(sourceType == BufferSourceType::EGL)
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,*(EGLImage*)data);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, format, imp()->sizeB.w(), imp()->sizeB.h(), 0, format, type, data);

    imp()->initialized = true;
}

void LTexture::LTexturePrivate::deleteTexture()
{
    glActiveTexture(GL_TEXTURE0 + unit);
    if(initialized)
    {
        glDeleteTextures(1, &id);
        initialized = false;
    }
}

const LSize &LTexture::sizeB() const
{
    return imp()->sizeB;
}

bool LTexture::initialized()
{
    return imp()->initialized;
}

GLuint LTexture::id()
{
    return imp()->id;
}

GLuint LTexture::unit()
{
    return imp()->unit;
}

GLenum LTexture::type()
{
    return imp()->type;
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

