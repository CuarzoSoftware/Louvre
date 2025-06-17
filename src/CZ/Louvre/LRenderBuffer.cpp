#include <CZ/Louvre/Private/LTexturePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/LRenderBuffer.h>
#include <CZ/Louvre/LCompositor.h>
#include <GLES2/gl2.h>
#include <CZ/Louvre/LLog.h>

using namespace Louvre;

LRenderBuffer::LRenderBuffer(const SkISize &sizeB) noexcept : LFramebuffer(RenderBuffer)
{
    m_texture.m_sourceType = LTexture::Framebuffer;
    m_texture.m_sizeB.fWidth = 1;
    m_texture.m_sizeB.fHeight = 1;
    setSizeB(sizeB);
}

LRenderBuffer::~LRenderBuffer() noexcept
{
    notifyDestruction();

    for (auto &pair : m_threadsMap)
        compositor()->imp()->addRenderBufferToDestroy(pair.first, pair.second);
}

void LRenderBuffer::setSizeB(const SkISize &sizeB) noexcept
{
    const SkISize newSize {sizeB.width() <= 0 ? 1 : sizeB.width(), sizeB.height() <= 0 ? 1 : sizeB.height()};

    if (m_texture.sizeB() != newSize)
    {
        m_texture.m_sizeB = newSize;

        m_rect.setXYWH(
            m_rect.x(),
            m_rect.y(),
            SkScalarRoundToInt(Float32(m_texture.m_sizeB.width()) / m_scale),
            SkScalarRoundToInt(Float32(m_texture.m_sizeB.height()) / m_scale));

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

void LRenderBuffer::setFramebufferDamage(const SkRegion *damage) noexcept
{
    L_UNUSED(damage);
}

CZTransform LRenderBuffer::transform() const noexcept
{
    return CZTransform::Normal;
}

Float32 LRenderBuffer::scale() const noexcept
{
    return m_scale;
}

SkISize LRenderBuffer::sizeB() const noexcept
{
    return m_texture.sizeB();
}

const SkIRect &LRenderBuffer::rect() const noexcept
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texture.sizeB().width(), m_texture.sizeB().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
