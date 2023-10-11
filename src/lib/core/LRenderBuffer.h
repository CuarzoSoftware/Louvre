#ifndef LRENDERBUFFER_H
#define LRENDERBUFFER_H

#include <LFramebuffer.h>

/**
 * @brief Custom render destination framebuffer
 *
 * The LRenderBuffer can be utilized to render to a framebuffer instead of an LOutput and then use that rendered content as a texture.\n
 * Essentially, it acts as a wrapper for an OpenGL framebuffer with additional functionality. You can obtain the OpenGL framebuffer ID using the id() method.
 * It can also be utilized with an LPainter by using LPainter::bindFramebuffer().
 *
 * @note A framebuffer possess properties such as position, size, and buffer scale. LPainter uses these properties to properly position and scale the rendered content.
 */
class Louvre::LRenderBuffer : public LFramebuffer
{
public:    
    /**
     * @brief Constructor for LRenderBuffer with specified size.
     *
     * This constructor creates an LRenderBuffer with the specified size in buffer coordinates.
     *
     * @param sizeB The size of the framebuffer in buffer coordinates.
     */
    LRenderBuffer(const LSize &sizeB);

    /// @cond OMIT
    LRenderBuffer(const LRenderBuffer&) = delete;
    LRenderBuffer& operator= (const LRenderBuffer&) = delete;
    /// @endcond

    /**
     * @brief Destructor for LRenderBuffer.
     */
    ~LRenderBuffer();

    /**
     * @brief Retrieve the OpenGL ID of the framebuffer.
     *
     * This method returns the OpenGL ID of the framebuffer associated with the LRenderBuffer.
     *
     * @return The OpenGL ID of the framebuffer.
     */
    GLuint id() const override;

    /**
     * @brief Retrieve the texture associated with the framebuffer.
     *
     * LRenderBuffers always have a single framebuffer, so 0 must always be passed as an argument.
     * The index is part of the LFramebuffer parent class, which may be used by special framebuffers like those of LOutput.
     *
     * @param index The index of the texture (default is 0).
     * @return A pointer to the texture associated with the framebuffer.
     */
    const LTexture *texture(Int32 index = 0) const override;

    /**
     * @brief Set the size of the framebuffer in buffer coordinates.
     *
     * @warning The existing framebuffer is destroyed and replaced by a new one each time this method is called, so it should be used with moderation.
     *
     * @param sizeB The new size of the framebuffer in buffer coordinates.
     */
    void setSizeB(const LSize &sizeB);

    /**
     * @brief Retrieve the size of the framebuffer in buffer coordinates.
     *
     * This method returns the size of the framebuffer in buffer coordinates.
     *
     * @return The size of the framebuffer.
     */
    const LSize &sizeB() const override;

    /**
     * @brief Retrieve the size of the framebuffer.
     *
     * This method returns the size of the framebuffer in surface coordinates. This is sizeB() / scale().
     *
     * @return The size of the framebuffer.
     */
    const LSize &size() const;

    /**
     * @brief Set the position of the framebuffer.
     *
     * This method sets the position of the framebuffer in surface coordinates.
     *
     * @param pos The new position of the framebuffer.
     */
    void setPos(const LPoint &pos);

    /**
     * @brief Retrieve the position of the framebuffer.
     *
     * This method returns the position of the framebuffer in surface coordinates.
     *
     * @return The position of the framebuffer.
     */
    const LPoint &pos() const;

    /**
     * @brief Retrieve the position and size of the framebuffer in surface coordinates.
     *
     * The size provided by this rect is equal to size().
     *
     * @return The position and size of the framebuffer in surface coordinates.
     */
    const LRect &rect() const override;

    /**
     * @brief Set the buffer scale to properly scale the rendered content.
     *
     * For example, framebuffers used in HiDPI displays should have a scale of 2 or greater.
     *
     * @param scale The buffer scale factor.
     */
    void setScale(Int32 scale) const;

    /**
     * @brief Retrieve the buffer scale of the framebuffer.
     *
     * This method returns the buffer scale factor of the framebuffer.
     *
     * @return The buffer scale factor.
     */
    Int32 scale() const override;

    Int32 buffersCount() const override;
    Int32 currentBufferIndex() const override;
    void setFramebufferDamage(const LRegion *damage) override;

LPRIVATE_IMP(LRenderBuffer)
};

#endif // LRENDERBUFFER_H
