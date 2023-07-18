#include <private/LRenderBufferPrivate.h>

GLuint LRenderBuffer::LRenderBufferPrivate::getTextureId(LOutput *output)
{
    if (!output)
        return 0;

    return outputsMap[output].textureId;
}
