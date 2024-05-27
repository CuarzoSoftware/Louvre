#ifndef LTRANSFORM_H
#define LTRANSFORM_H

#include <LNamespaces.h>

namespace Louvre
{
    /**
     * @brief Enumeration for Framebuffer Transformations
     *
     * This enumeration defines different transformations that can be applied to a framebuffer.
     * These transformations include rotations and flips for adjusting the orientation of the framebuffer.
     */
    enum class LTransform : Int32
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

    /**
     * @brief Checks if the transformation results in swapping the width and height.
     *
     * @param transform The transformation to check.
     * @return `true` if the transformation includes a 90° or 270° rotation, `false` otherwise.
     */
    static inline constexpr bool is90Transform(LTransform transform) noexcept
    {
        return static_cast<Int32>(transform) & static_cast<Int32>(LTransform::Rotated90);
    }

    /**
     * @brief Required transform to transition from transform 'a' to 'b'
     *
     * @param a The initial transform.
     * @param b The target transform.
     */
    static inline constexpr LTransform requiredTransform(LTransform a, LTransform b) noexcept
    {
        const Int32 bitmask { static_cast<Int32>(LTransform::Rotated270) };
        const Int32 flip { (static_cast<Int32>(a) & ~bitmask) ^ (static_cast<Int32>(b) & ~bitmask) };
        Int32 rotation;

        if (flip)
            rotation = ((static_cast<Int32>(b) & bitmask) + (static_cast<Int32>(a) & bitmask)) & bitmask;
        else
        {
            rotation = (static_cast<Int32>(b) & bitmask) - (static_cast<Int32>(a) & bitmask);

            if (rotation < 0)
                rotation += 4;
        }

        return static_cast<LTransform>(flip | rotation);
    }
}

#endif // LTRANSFORM_H
