#include <private/LRenderBufferPrivate.h>

GLuint LRenderBuffer::LRenderBufferPrivate::getTextureId()
{
    return threadsMap[std::this_thread::get_id()].textureId;
}
