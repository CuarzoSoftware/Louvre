#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

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
    void increaseSerial();
    bool pendingDelete = false;

    std::list<LTexture*>::iterator compositorLink;

    // List of texture views using it
    std::list<LTextureView*> textureViews;

    // Wrapper for a native OpenGL ES 2.0 texture.
    bool setDataB(GLuint textureId, GLenum target, UInt32 format, const LSize &size, LOutput *output);
    GLuint nativeId = 0;
    GLenum nativeTarget = 0;
    LOutput *nativeOutput = nullptr;
};

#endif // LTEXTUREPRIVATE_H
