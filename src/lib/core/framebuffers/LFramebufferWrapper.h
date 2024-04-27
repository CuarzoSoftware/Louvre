#ifndef LGLFRAMEBUFFER_H
#define LGLFRAMEBUFFER_H

#include <LFramebuffer.h>
#include <LRect.h>

/**
 * @brief Wrapper for a native OpenGL framebuffer.
 *
 * This class facilitates the usage of an existing native OpenGL framebuffer with LPainter.\n
 * It requires providing the framebuffer ID, its buffer size, scaling factor and position.
 *
 * @note Native OpenGL framebuffers are not shared across threads, and thus cannot be utilized from all LOutputs.\n
 *       If a framebuffer is needed for use from any thread, consider using LRenderBuffer instead.
 */
class Louvre::LFramebufferWrapper final : public LFramebuffer
{
public:
    /**
     * @brief Constructs an LGLFramebuffer object.
     *
     * @param id The ID of the native OpenGL framebuffer.
     * @param sizeB The buffer size of the framebuffer.
     * @param pos The position of the framebuffer in the global compositor-coordinates space.
     * @param scale The scaling factor of the framebuffer (default is 1.0).
     */
    LFramebufferWrapper(GLuint id,
                   const LSize &sizeB,
                   const LPoint &pos = LPoint(0,0),
                   Float32 scale = 1.f) noexcept :
        m_sizeB(sizeB),
        m_fbId(id),
        m_scale(scale)
    {
        m_type = Wrapper;
        m_rect.setPos(pos);
        updateDimensions();
    }

    /**
     * @brief Sets the ID of the native OpenGL framebuffer.
     *
     * @param id The ID of the framebuffer.
     */
    void setId(GLuint id) noexcept
    {
        m_fbId = id;
    }

    /**
     * @brief Sets the buffer size of the framebuffer.
     *
     * @param sizeB The buffer size.
     */
    void setSizeB(const LSize &sizeB) noexcept
    {
        m_sizeB = sizeB;
        updateDimensions();
    }

    /**
     * @brief Sets the scaling factor of the framebuffer.
     *
     * @param scale The scaling factor.
     */
    void setScale(Float32 scale) noexcept
    {
        m_scale = scale;
        updateDimensions();
    }

    /**
     * @brief Sets the position of the framebuffer in the global compositor-coordinates space.
     *
     * @param pos The position.
     */
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
    LTexture *texture(Int32) const noexcept override { return nullptr; };
    void setFramebufferDamage(const LRegion *) noexcept override {};
    virtual LTransform transform() const noexcept override { return LTransform::Normal; };

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
