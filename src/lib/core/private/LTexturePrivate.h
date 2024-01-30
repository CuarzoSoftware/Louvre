#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

#include <GL/gl.h>
#include <LTexture.h>
#include <LSize.h>

using namespace Louvre;

LPRIVATE_CLASS(LTexture)
    LTexture *texture;
    void deleteTexture();

    BufferSourceType sourceType                         = CPU;
    LSize sizeB;
    UInt32 format                                       = 0;
    void *graphicBackendData                            = nullptr;

    // Increases each time the texture is modified
    UInt32 serial                                       = 0;
    bool pendingDelete = false;

    // List of texture views using it
    std::list<LTextureView*> textureViews;

    // Wrapper for a native OpenGL ES 2.0 texture.
    GLuint nativeId = 0;
    GLenum nativeTarget = 0;
    LOutput *nativeOutput = nullptr;

    // Utility functions    
    inline bool setDataB(GLuint textureId, GLenum target, UInt32 format, const LSize &size, LOutput *output)
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

    inline static void setTextureParams(GLuint textureId, GLenum target, GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter)
    {
        glBindTexture(target, textureId);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
    }

    inline static void readPixels(const LRect &src, const LPoint &dstOffset, Int32 dstWidth, GLenum format, GLenum type, UChar8 *buffer)
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
};

#endif // LTEXTUREPRIVATE_H
