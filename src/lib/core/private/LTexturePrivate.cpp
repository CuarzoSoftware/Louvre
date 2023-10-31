#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>

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

void LTexture::LTexturePrivate::setTextureParams(GLuint textureId, GLenum target, GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter)
{
    glBindTexture(target, textureId);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
}

void LTexture::LTexturePrivate::readPixels(const LRect &src, const LPoint &dstOffset, Int32 dstWidth, GLenum format, GLenum type, UChar8 *buffer)
{
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, dstWidth);
    glPixelStorei(GL_PACK_SKIP_PIXELS, dstOffset.x());
    glPixelStorei(GL_PACK_SKIP_ROWS, dstOffset.y());
    glReadPixels(src.x(), src.y(), src.w(), src.h(), format, type, buffer);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
}
