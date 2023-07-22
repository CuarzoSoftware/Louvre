#include <private/LRenderBufferPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LCompositor.h>
#include <GLES2/gl2.h>
#include <LLog.h>

LRenderBuffer::LRenderBuffer(const LSize &sizeB)
{
    m_imp = new LRenderBufferPrivate();
    imp()->texture.imp()->sourceType = LTexture::Framebuffer;
    imp()->texture.imp()->format = DRM_FORMAT_ARGB8888;
    imp()->texture.imp()->graphicBackendData = this;
    setSizeB(sizeB);
}

LRenderBuffer::~LRenderBuffer()
{
    for (auto &pair : imp()->threadsMap)
        if (pair.second.textureId)
            compositor()->imp()->addRenderBufferToDestroy(pair.second);

    delete m_imp;
}

void LRenderBuffer::setSizeB(const LSize &sizeB)
{
    if (imp()->texture.imp()->sizeB != sizeB)
    {
        imp()->texture.imp()->sizeB = sizeB;

        imp()->rect.setSize(sizeB/imp()->scale);

        for (auto &pair : imp()->threadsMap)
            if (pair.second.textureId)
                compositor()->imp()->addRenderBufferToDestroy(pair.second);

        imp()->threadsMap.clear();

    }
}

const LTexture *LRenderBuffer::texture(Int32 index) const
{
    L_UNUSED(index);
    return &imp()->texture;
}

void LRenderBuffer::setFramebufferDamageC(const LRegion *damage)
{
    L_UNUSED(damage);
}

void LRenderBuffer::setScale(Int32 scale) const
{
    if (imp()->scale != scale)
    {
        imp()->rect.setSize(sizeB()/scale);
        imp()->scale = scale;
    }
}

void LRenderBuffer::setPos(const LPoint &pos)
{
    imp()->rect.setPos(pos);
}

Int32 LRenderBuffer::scale() const
{
    return imp()->scale;
}

const LSize &LRenderBuffer::sizeB() const
{
    return imp()->texture.sizeB();
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
        glBindTexture(GL_TEXTURE_2D, data.textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
