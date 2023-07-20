#ifndef LRENDERBUFFERPRIVATE_H
#define LRENDERBUFFERPRIVATE_H

#include <LRenderBuffer.h>
#include <LTexture.h>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LRenderBuffer)
    LTexture texture;
    Int32 scale = 1;
    LRect rect;

    struct OutputData
    {
        GLuint textureId = 0;
        GLuint framebufferId = 0;
    };

    GLuint getTextureId(LOutput *output);
    std::map<LOutput*, OutputData>outputsMap;

    std::list<LRenderBuffer*>::iterator compositorLink;
};

#endif // LRENDERBUFFERPRIVATE_H
