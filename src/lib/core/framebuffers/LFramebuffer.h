#ifndef LFRAMEBUFFER_H
#define LFRAMEBUFFER_H

#include <LObject.h>

/**
 * @brief Base class for LPainter framebuffers.
 *
 * The LFramebuffer abstract class defines an interface for creating framebuffers or groups of framebuffers compatible with LPainter.
 *
 * @see LPainter::bindFramebuffer() for usage instructions.
 * @see LRenderBuffer
 * @see LFramebufferWrapper
 * @see LOutputFramebuffer
 */
class Louvre::LFramebuffer : public LObject
{
public:

    /**
     * @brief Types of framebuffer.
     */
    enum Type
    {
        /// LOutputFramebuffer
        Output,

        /// LRenderBuffer
        RenderBuffer,

        /// LFramebufferWrapper
        Wrapper
    };

    /**
     * @brief Get the scale by which the framebuffer dimensions must be interpreted.
     *
     * This method must return the scale factor used to interpret the dimensions of the framebuffer.\n
     * For example, HiDPI pixmaps/displays may have a scale of 2, whereas low DPI pixmaps/displays may have a scale of 1.
     *
     * @returns The scale factor for the framebuffer.
     */
    virtual Float32 scale() const = 0;

    /**
     * @brief Get the size of the framebuffer in buffer coordinates.
     *
     * This method must return the size of the framebuffer in buffer coordinates.
     *
     * @returns The size of the framebuffer in buffer coordinates.
     */
    virtual const LSize &sizeB() const = 0;

    /**
     * @brief Get the position and size of the framebuffer in surface coordinates.
     *
     * This method must return the position and size of the framebuffer in surface coordinates.
     *
     * @returns The rect representing the position and size of the framebuffer in surface coordinates.
     */
    virtual const LRect &rect() const = 0;

    /**
     * @brief Get the OpenGL framebuffer ID.
     *
     * This method must return the current OpenGL framebuffer ID associated with the instance.
     *
     * @returns The OpenGL framebuffer ID.
     */
    virtual GLuint id() const  = 0;

    /**
     * @brief Get the number of internal framebuffers.
     *
     * This method is used in implementations like LOutputFramebuffer, which may contain more than one
     * internal framebuffer associated with an LFramebuffer, for example, when double or triple buffering is used.
     *
     * @return The number of internal framebuffers.
     */
    virtual Int32 buffersCount() const = 0;

    /**
     * @brief Get the index of the current internal framebuffer
     *
     * This method must return the index of the current internal framebuffer (where the rendered content will be stored).
     *
     * @returns The index of the framebuffer used for rendering.
     */
    virtual Int32 currentBufferIndex() const = 0;

    /**
     * @brief Get the OpenGL texture ID of a specific framebuffer index.
     *
     * This method must return the OpenGL texture ID associated with the specified framebuffer index.
     *
     * @param index The index of the framebuffer.
     * @returns The OpenGL texture ID of the specified framebuffer index.
     */
    virtual LTexture *texture(Int32 index = 0) const = 0;

    /**
     * @brief Set the damaged region
     *
     * This method sets the region in surface coordinates that indicates the parts of the framebuffer
     * that have changed since the last painting was performed.
     *
     * @param damage A pointer to the LRegion object representing the changed region.
     */
    virtual void setFramebufferDamage(const LRegion *damage) = 0;

    /**
     * @brief Get the framebuffer transform.
     */
    virtual LTransform transform() const = 0;

    /**
     * @brief Gets the type of the framebuffer.
     */
    Type type() const noexcept
    {
        return m_type;
    }

protected:
    Type m_type;
};

#endif // LFRAMEBUFFER_H
