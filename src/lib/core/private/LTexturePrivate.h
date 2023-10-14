#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

#include <LTexture.h>
#include <LSize.h>

using namespace Louvre;

LPRIVATE_CLASS(LTexture)
    void deleteTexture(LTexture *texture);

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
};

#endif // LTEXTUREPRIVATE_H
