#ifndef LGLFRAMEBUFFER_H
#define LGLFRAMEBUFFER_H

#include <LFramebuffer.h>
#include <LRect.h>

// TODO add doc
// Wrapper for raw GL fb
class Louvre::LGLFramebuffer final : public LFramebuffer
{
public:
    LGLFramebuffer(GLuint id,
                   const LSize &sizeB,
                   const LPoint &pos = LPoint(0,0),
                   Float32 scale = 1.f) noexcept :
        m_sizeB(sizeB),
        m_fbId(id),
        m_scale(scale)
    {
        m_type = Render;
        m_rect.setPos(pos);
        updateDimensions();
    }

    void setId(GLuint id) noexcept
    {
        m_fbId = id;
    }

    void setSizeB(const LSize &sizeB) noexcept
    {
        m_sizeB = sizeB;
        updateDimensions();
    }

    void setScale(Float32 scale) noexcept
    {
        m_scale = scale;
        updateDimensions();
    }

    void setPos(const LPoint &pos) noexcept
    {
        m_rect.setPos(pos);
    }

    Float32 scale() const noexcept override { return m_scale; };
    const LSize &sizeB() const noexcept override { return m_sizeB; };
    const LRect &rect() const noexcept override {  return m_rect; };
    GLuint id() const noexcept override { return m_fbId; };
    Int32 buffersCount() const noexcept override { return 1; };
    Int32 currentBufferIndex() const noexcept override { return 0; };
    const LTexture *texture(Int32) const noexcept override { return nullptr; };
    void setFramebufferDamage(const LRegion *) noexcept override {};
    virtual Transform transform() const noexcept override { return Normal; };

private:
    void updateDimensions() noexcept
    {
        if (m_sizeB.w() < 0)
            m_sizeB.setW(0);

        if (m_sizeB.h() < 0)
            m_sizeB.setH(0);

        if (m_scale < 0.25f)
            m_scale = 0.25f;

        m_rect.setW(Float32(m_sizeB.w())/m_scale);
        m_rect.setH(Float32(m_sizeB.h())/m_scale);
    }
    LRect m_rect;
    LSize m_sizeB;
    GLuint m_fbId;
    Float32 m_scale;
};

#endif // LGLFRAMEBUFFER_H
