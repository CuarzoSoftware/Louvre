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

    void deleteTexture(LTexture *texture);

    LCompositor *compositor                             = nullptr;
    BufferSourceType sourceType                         = CPU;
    GLuint unit                                         = 0;
    LSize sizeB;
    UInt32 format                                       = 0;
    void *graphicBackendData                            = nullptr;

    // Increases each time the texture is modified
    UInt32 serial                                       = 0;
    void increaseSerial();

    bool pendingDelete = false;
};

#endif // LTEXTUREPRIVATE_H
