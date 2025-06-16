#ifndef LGLFRAMEBUFFER_H
#define LGLFRAMEBUFFER_H

#include <LFramebuffer.h>
#include <CZ/skia/core/SkRect.h>

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
    LFramebufferWrapper(GLuint id, SkISize sizeB, SkIPoint pos = SkIPoint(0,0),
                        Float32 scale = 1.f) noexcept : LFramebuffer(Wrapper),
        m_sizeB(sizeB),
        m_fbId(id),
        m_scale(scale)
    {
        m_rect.offsetTo(pos.x(), pos.y());
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
    void setSizeB(const SkISize &sizeB) noexcept
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
    void setPos(SkIPoint pos) noexcept
    {
        m_rect.offsetTo(pos.x(), pos.y());
    }

    Float32 scale() const noexcept override { return m_scale; };
    SkISize sizeB() const noexcept override { return m_sizeB; };
    const SkIRect &rect() const noexcept override {  return m_rect; };
    GLuint id() const noexcept override { return m_fbId; };
    Int32 buffersCount() const noexcept override { return 1; };
    Int32 currentBufferIndex() const noexcept override { return 0; };
    LTexture *texture(Int32) const noexcept override { return nullptr; };
    void setFramebufferDamage(const SkRegion *) noexcept override {};
    virtual CZTransform transform() const noexcept override { return CZTransform::Normal; };

private:
    void updateDimensions() noexcept
    {
        if (m_sizeB.width() < 0)
            m_sizeB.fWidth = 0;

        if (m_sizeB.height() < 0)
            m_sizeB.fHeight = 0;

        if (m_scale < 0.25f)
            m_scale = 0.25f;

        m_rect.setXYWH(
            m_rect.x(),
            m_rect.y(),
            Float32(m_sizeB.width())/m_scale,
            Float32(m_sizeB.height())/m_scale);
    }
    SkIRect m_rect;
    SkISize m_sizeB;
    GLuint m_fbId;
    Float32 m_scale;
};

#endif // LGLFRAMEBUFFER_H
