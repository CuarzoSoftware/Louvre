#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LRenderBuffer.h>
#include <LCompositor.h>
#include <GLES2/gl2.h>
#include <LLog.h>

using namespace Louvre;

LRenderBuffer::LRenderBuffer(const LSize &sizeB) noexcept : LFramebuffer(RenderBuffer)
{
    m_texture.m_sourceType = LTexture::Framebuffer;
    m_texture.m_sizeB.setW(1);
    m_texture.m_sizeB.setH(1);
    setSizeB(sizeB);
}

LRenderBuffer::~LRenderBuffer() noexcept
{
    notifyDestruction();

    for (auto &pair : m_threadsMap)
        compositor()->imp()->addRenderBufferToDestroy(pair.first, pair.second);
}

void LRenderBuffer::setSizeB(const LSize &sizeB) noexcept
{
    const LSize newSize {sizeB.w() <= 0 ? 1 : sizeB.w(), sizeB.h() <= 0 ? 1 : sizeB.h()};

    if (m_texture.sizeB() != newSize)
    {
        m_texture.m_sizeB = newSize;

        m_rect.setW(roundf(Float32(m_texture.m_sizeB.w()) / m_scale));
        m_rect.setH(roundf(Float32(m_texture.m_sizeB.h()) / m_scale));

        for (auto &pair : m_threadsMap)
            compositor()->imp()->addRenderBufferToDestroy(pair.first, pair.second);

        m_texture.reset();
        m_threadsMap.clear();
    }
}

LTexture *LRenderBuffer::texture(Int32 index) const noexcept
{
    L_UNUSED(index);
    return &m_texture;
}

void LRenderBuffer::setFence() noexcept
{
    m_texture.setFence();
}

void LRenderBuffer::setFramebufferDamage(const LRegion *damage) noexcept
{
    L_UNUSED(damage);
}

LTransform LRenderBuffer::transform() const noexcept
{
    return LTransform::Normal;
}

Float32 LRenderBuffer::scale() const noexcept
{
    return m_scale;
}

const LSize &LRenderBuffer::sizeB() const noexcept
{
    return m_texture.sizeB();
}

const LRect &LRenderBuffer::rect() const noexcept
{
    return m_rect;
}

GLuint LRenderBuffer::id() const noexcept
{
    ThreadData &data { m_threadsMap[std::this_thread::get_id()] };

    if (!data.framebufferId)
    {
        glGenFramebuffers(1, &data.framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, data.framebufferId);

        if (!m_texture.initialized())
        {
            GLuint tex;
            glGenTextures(1, &tex);
            LTexture::LTexturePrivate::setTextureParams(tex, GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture.sizeB().w(), m_texture.sizeB().h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            m_texture.m_sourceType = LTexture::GL;
            m_texture.setDataFromGL(tex, GL_TEXTURE_2D, DRM_FORMAT_ABGR8888, m_texture.sizeB(), true);
            m_texture.m_sourceType = LTexture::Framebuffer;
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.id(nullptr), 0);
    }

    return data.framebufferId;
}

Int32 LRenderBuffer::buffersCount() const noexcept
{
    return 1;
}

Int32 LRenderBuffer::currentBufferIndex() const noexcept
{
    return 0;
}
