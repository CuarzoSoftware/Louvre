#include <private/LRenderBufferPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LCompositor.h>
#include <GLES2/gl2.h>
#include <LLog.h>

LRenderBuffer::LRenderBuffer(const LSize &sizeB) : LPRIVATE_INIT_UNIQUE(LRenderBuffer)
{
    imp()->texture.imp()->sourceType = LTexture::Framebuffer;
    imp()->texture.imp()->format = DRM_FORMAT_BGRA8888;
    imp()->texture.imp()->graphicBackendData = this;
    setSizeB(sizeB);
}

LRenderBuffer::~LRenderBuffer()
{
    for (auto &pair : imp()->threadsMap)
        if (pair.second.textureId)
            compositor()->imp()->addRenderBufferToDestroy(pair.first, pair.second);
}

void LRenderBuffer::setSizeB(const LSize &sizeB)
{
    if (imp()->texture.imp()->sizeB != sizeB)
    {
        imp()->texture.imp()->sizeB = sizeB;

        imp()->rect.setSize(sizeB/imp()->scale);

        for (auto &pair : imp()->threadsMap)
            if (pair.second.textureId)
                compositor()->imp()->addRenderBufferToDestroy(pair.first, pair.second);

        imp()->threadsMap.clear();
    }
}

const LTexture *LRenderBuffer::texture(Int32 index) const
{
    L_UNUSED(index);
    return &imp()->texture;
}

void LRenderBuffer::setFramebufferDamage(const LRegion *damage)
{
    L_UNUSED(damage);
}

LFramebuffer::Transform LRenderBuffer::transform() const
{
    return LFramebuffer::Normal;
}

void LRenderBuffer::setScale(Float32 scale) const
{
    if (scale < 1.f)
        return;

    if (imp()->scale != scale)
    {
        imp()->rect.setW(roundf(Float32(sizeB().w())/scale));
        imp()->rect.setH(roundf(Float32(sizeB().h())/scale));
        imp()->scale = scale;
    }
}

void LRenderBuffer::setPos(const LPoint &pos)
{
    imp()->rect.setPos(pos);
}

const LPoint &LRenderBuffer::pos() const
{
    return imp()->rect.pos();
}

Float32 LRenderBuffer::scale() const
{
    return imp()->scale;
}

const LSize &LRenderBuffer::sizeB() const
{
    return imp()->texture.sizeB();
}

const LSize &LRenderBuffer::size() const
{
    return imp()->rect.size();
}

const LRect &LRenderBuffer::rect() const
{
    return imp()->rect;
}

GLuint LRenderBuffer::id() const
{
    LRenderBufferPrivate::ThreadData &data = imp()->threadsMap[std::this_thread::get_id()];

    if (!data.textureId)
    {
        glGenFramebuffers(1, &data.framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, data.framebufferId);
        glGenTextures(1, &data.textureId);
        LTexture::LTexturePrivate::setTextureParams(data.textureId, GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imp()->texture.sizeB().w(), imp()->texture.sizeB().h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.textureId, 0);
    }

    return data.framebufferId;
}

Int32 LRenderBuffer::buffersCount() const
{
    return 1;
}

Int32 LRenderBuffer::currentBufferIndex() const
{
    return 0;
}
