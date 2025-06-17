#ifndef LRENDERBUFFER_H
#define LRENDERBUFFER_H

#include <CZ/Louvre/LTexture.h>
#include <CZ/Louvre/LFramebuffer.h>
#include <thread>

/**
 * @brief Represents a custom render destination framebuffer.
 *
 * The LRenderBuffer provides a means to render to a framebuffer instead of an LOutput, allowing the resulting content to be used as a texture.\n
 * It can be employed with an LPainter via LPainter::bindFramebuffer().
 *
 * @note The framebuffer possesses properties such as position, size, and scale factor. These properties are utilized by LPainter to appropriately position and scale the rendered content.
 */
class Louvre::LRenderBuffer final : public LFramebuffer
{
public:   
    /**
     * @brief Constructor for LRenderBuffer with specified size.
     *
     * This constructor creates an LRenderBuffer with the specified size in buffer coordinates.
     *
     * @param sizeB The size of the framebuffer in buffer coordinates.
     */
    LRenderBuffer(const SkISize &sizeB) noexcept;

    LCLASS_NO_COPY(LRenderBuffer)

    /**
     * @brief Destructor for LRenderBuffer.
     */
    ~LRenderBuffer() noexcept;

    /**
     * @brief Retrieve the OpenGL ID of the framebuffer.
     *
     * This method returns the OpenGL ID of the framebuffer associated with the LRenderBuffer.
     *
     * @return The OpenGL ID of the framebuffer.
     */
    GLuint id() const noexcept override;

    /**
     * @brief Retrieve the texture associated with the framebuffer.
     *
     * LRenderBuffers always have a single framebuffer, so 0 must always be passed as an argument.
     * The index is part of the LFramebuffer parent class, which may be used by special framebuffers like those of LOutput.
     *
     * @param index The index of the texture (default is 0).
     * @return A pointer to the texture associated with the framebuffer.
     */
    LTexture *texture(Int32 index = 0) const noexcept override;

    /**
     * @brief Sets a fence for synchronization after rendering operations.
     *
     * This method should be called after rendering is complete on the renderbuffer.
     *
     * It ensures that all rendering commands are finished before the texture is used
     * elsewhere, preventing potential data inconsistencies or rendering artifacts.
     */
    void setFence() noexcept;

    /**
     * @brief Set the size of the framebuffer in buffer coordinates.
     *
     * @warning The existing framebuffer is destroyed and replaced by a new one each time this method is called, so it should be used with moderation.
     *
     * @param sizeB The new size of the framebuffer in buffer coordinates.
     */
    void setSizeB(const SkISize &sizeB) noexcept;

    /**
     * @brief Retrieve the size of the framebuffer in buffer coordinates.
     *
     * This method returns the size of the framebuffer in buffer coordinates.
     *
     * @return The size of the framebuffer.
     */
    SkISize sizeB() const noexcept override;

    /**
     * @brief Retrieve the size of the framebuffer.
     *
     * This method returns the size of the framebuffer in surface coordinates. This is sizeB() / scale().
     *
     * @return The size of the framebuffer.
     */
    SkISize size() const noexcept
    {
        return m_rect.size();
    }

    /**
     * @brief Set the position of the framebuffer.
     *
     * This method sets the position of the framebuffer in surface coordinates.
     *
     * @param pos The new position of the framebuffer.
     */
    void setPos(SkIPoint pos) noexcept
    {
        m_rect.offsetTo(pos.x(), pos.y());
    }

    /**
     * @brief Retrieve the position of the framebuffer.
     *
     * This method returns the position of the framebuffer in surface coordinates.
     *
     * @return The position of the framebuffer.
     */
    SkIPoint pos() const noexcept
    {
        return m_rect.topLeft();
    }

    /**
     * @brief Retrieve the position and size of the framebuffer in surface coordinates.
     *
     * The size provided by this rect is equal to size().
     *
     * @return The position and size of the framebuffer in surface coordinates.
     */
    const SkIRect &rect() const noexcept override;

    /**
     * @brief Set the buffer scale to properly scale the rendered content.
     *
     * For example, framebuffers used in HiDPI displays should have a scale of 2 or greater.
     *
     * @param scale The buffer scale factor.
     */
    void setScale(Float32 scale) noexcept
    {
        if (scale < 0.25f)
            scale = 0.25;

        if (m_scale != scale)
        {
            m_rect.setWH(
                roundf(Float32(m_texture.sizeB().width())/scale),
                roundf(Float32(m_texture.sizeB().height())/scale)
            );
            m_scale = scale;
        }
    }

    Float32 scale() const noexcept override;
    Int32 buffersCount() const noexcept override;
    Int32 currentBufferIndex() const noexcept override;
    void setFramebufferDamage(const SkRegion *damage) noexcept override;
    CZTransform transform() const noexcept override;

private:
    friend class LCompositor;
    struct ThreadData
    {
        GLuint framebufferId = 0;
    };
    mutable LTexture m_texture { true };
    SkIRect m_rect { 0, 0, 0, 0 };
    Float32 m_scale { 1.f };
    mutable std::unordered_map<std::thread::id, ThreadData> m_threadsMap;
};

#endif // LRENDERBUFFER_H
