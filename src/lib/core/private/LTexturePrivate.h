#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

#include <LTexture.h>
#include <LSize.h>

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
};

#endif // LTEXTUREPRIVATE_H
