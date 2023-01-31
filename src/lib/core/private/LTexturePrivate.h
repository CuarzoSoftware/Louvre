#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

#include <LTexture.h>
#include <LSize.h>

class Louvre::LTexture::LTexturePrivate
{
public:
    LTexturePrivate()                                   = default;
    ~LTexturePrivate()                                  = default;

    LTexturePrivate(const LTexturePrivate&)             = delete;
    LTexturePrivate& operator= (const LTexturePrivate&) = delete;

    void deleteTexture();

    LSize sizeB;
    GLuint id                                           = 0;
    GLuint unit                                         = 0;
    GLenum type                                         = GL_UNSIGNED_BYTE;
    bool initialized                                    = false;
    BufferSourceType sourceType                         = SHM;
    GLenum format                                       = 0;
};

#endif // LTEXTUREPRIVATE_H
