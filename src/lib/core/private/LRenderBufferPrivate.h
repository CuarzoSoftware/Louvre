#ifndef LRENDERBUFFERPRIVATE_H
#define LRENDERBUFFERPRIVATE_H

#include <LRenderBuffer.h>
#include <LTexture.h>
#include <map>
#include <thread>

using namespace Louvre;

LPRIVATE_CLASS(LRenderBuffer)
    LTexture texture;
    Int32 scale = 1;
    LRect rect;

    struct ThreadData
    {
        GLuint textureId = 0;
        GLuint framebufferId = 0;
    };

    GLuint getTextureId();
    std::map<std::thread::id, ThreadData>threadsMap;

};

#endif // LRENDERBUFFERPRIVATE_H
