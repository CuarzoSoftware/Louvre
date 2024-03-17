#ifndef LFRAMEBUFFER_H
#define LFRAMEBUFFER_H

#include <LObject.h>

/**
 * @brief Base class for creating framebuffers.
 *
 * @note This class is primarily intended for internal use by Louvre and may not be directly useful to you.
 *
 * The LFramebuffer abstract class provides an interface for creating framebuffers or "groups of framebuffers" that can be used with LPainter.\n
 * Framebuffers serve as destinations where rendering can be performed and stored.
 *
 * For example, each LOutput can have one, two, or more framebuffers (LOutputFramebuffer) that are displayed on screen, or each LSceneView also have its own framebuffer.
 *
 * If you need to render into a framebuffer and then use it as a texture, consider using the LRenderBuffer class instead.
 *
 * @see LPainter::bindFramebuffer() for usage details.
 */
class Louvre::LFramebuffer : public LObject
{
public:
   
    enum Type
    {
        Output,
        Render
    };
   
    inline Type type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Enumeration for Framebuffer Transformations
     *
     * This enumeration defines different transformations that can be applied to a framebuffer.
     * These transformations include rotations and flips for adjusting the orientation of the framebuffer.
     */
    enum Transform : Int32
    {
        /// No transformation
        Normal = 0,

        /// Rotate 90 degrees counter-clockwise
        Rotated90 = 1,

        /// Rotate 180 degrees counter-clockwise
        Rotated180 = 2,

        /// Rotate 270 degrees counter-clockwise
        Rotated270 = 3,

        /// Flipped (swap left and right sides)
        Flipped = 4,

        /// Flip and rotate 90 degrees counter-clockwise
        Flipped90 = 5,

        /// Flip and rotate 180 degrees counter-clockwise
        Flipped180 = 6,

        /// Flip and rotate 270 degrees counter-clockwise
        Flipped270 = 7
    };

    static inline constexpr bool is90Transform(Transform transform) noexcept
    {
        return transform & Rotated90;
    }

    static inline constexpr Transform requiredTransform(Transform from, Transform to) noexcept
    {
        const Int32 bitmask { Rotated270 };
        const Int32 flip { (from & ~bitmask) ^ (to & ~bitmask) };
        Int32 rotation;

        if (flip)
            rotation = ((to & bitmask) + (from & bitmask)) & bitmask;
        else
        {
            rotation = (to & bitmask) - (from & bitmask);

            if (rotation < 0)
                rotation += 4;
        }

        return static_cast<LFramebuffer::Transform>(flip | rotation);
    }

    /**
     * @brief Destructor for the LFramebuffer class.
     */
    virtual ~LFramebuffer() noexcept = default;

    /**
     * @brief Get the scale by which the framebuffer dimensions must be interpreted.
     *
     * This method must return the scale factor used to interpret the dimensions of the framebuffer.
     * For example, HiDPI pixmaps/displays may have a scale of 2, whereas low DPI pixmaps/displays may have a scale of 1.
     *
     * @returns The scale factor for the framebuffer.
     */
    virtual Float32 scale() const noexcept = 0;

    /**
     * @brief Get the size of the framebuffer in buffer coordinates.
     *
     * This method must return the size of the framebuffer in buffer coordinates.
     *
     * @returns The size of the framebuffer in buffer coordinates.
     */
    virtual const LSize &sizeB() const noexcept = 0;

    /**
     * @brief Get the position and size of the framebuffer in surface coordinates.
     *
     * This method must return the position and size of the framebuffer in surface coordinates.
     *
     * @returns The rect representing the position and size of the framebuffer in surface coordinates.
     */
    virtual const LRect &rect() const noexcept = 0;

    /**
     * @brief Get the OpenGL framebuffer ID.
     *
     * This method must return the current OpenGL framebuffer ID associated with the instance.
     *
     * @returns The OpenGL framebuffer ID.
     */
    virtual GLuint id() const noexcept = 0;

    /**
     * @brief Get the number of framebuffers represented by this instance.
     *
     * This method must return the number of framebuffers represented by this instance.
     *
     * @returns The number of framebuffers.
     */
    virtual Int32 buffersCount() const noexcept = 0;

    /**
     * @brief Get the index of the framebuffer where the rendering is stored.
     *
     * This method must return the index of the framebuffer where rendering must be performed/stored.
     *
     * @returns The index of the framebuffer used for rendering.
     */
    virtual Int32 currentBufferIndex() const noexcept = 0;

    /**
     * @brief Get the OpenGL texture ID of a specific framebuffer index.
     *
     * This method must return the OpenGL texture ID associated with the specified framebuffer index.
     *
     * @param index The index of the framebuffer.
     * @returns The OpenGL texture ID of the specified framebuffer index.
     */
    virtual const LTexture *texture(Int32 index = 0) const noexcept = 0;

    /**
     * @brief Set the damaged region
     *
     * This method sets the region in surface coordinates that indicates the parts of the framebuffer
     * that have changed since the last painting was performed.
     *
     * @param damage A pointer to the LRegion object representing the changed region.
     */
    virtual void setFramebufferDamage(const LRegion *damage) noexcept = 0;

    /**
     * @brief Get the framebuffer transformation.
     *
     * This method must return the current framebuffer transformation applied.
     *
     * @return The framebuffer transformation.
     */
    virtual Transform transform() const noexcept = 0;
   
protected:
    Type m_type;
};

#endif // LFRAMEBUFFER_H
