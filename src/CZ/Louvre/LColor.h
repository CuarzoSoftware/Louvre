#ifndef LCOLOR_H
#define LCOLOR_H

#include <CZ/Louvre/LNamespaces.h>

namespace Louvre
{
    /**
     * @brief RGB color with floating-point components.
     *
     * The LRGBF struct defines an RGB color with three floating-point components (r, g, b).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBF
    {
        /// The red component of the RGB color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGB color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGB color (range: 0.0 to 1.0).
        Float32 b;

        constexpr bool operator==(const LRGBF &other) const noexcept
        {
            return r == other.r && g == other.g && b == other.b;
        }

        constexpr bool operator!=(const LRGBF &other) const noexcept
        {
            return r != other.r || g != other.g || b != other.b;
        }
    };

    /**
     * @brief RGBA color with floating-point components.
     *
     * The LRGBAF struct defines an RGBA color with four floating-point components (r, g, b, a).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBAF
    {
        /// The red component of the RGBA color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGBA color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGBA color (range: 0.0 to 1.0).
        Float32 b;

        /// The alpha component of the RGBA color (range: 0.0 to 1.0).
        Float32 a;

        constexpr bool operator==(const LRGBAF &other) const noexcept
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        constexpr bool operator!=(const LRGBAF &other) const noexcept
        {
            return r != other.r || g != other.g || b != other.b || a != other.a;
        }
    };

    /**
     * @brief Color blending function
     *
     * OpenGL blend function. Refer to the documentation
     * of [glBlendFuncSeparate()](https://docs.gl/es2/glBlendFuncSeparate) for more information.
     */
    struct LBlendFunc
    {
        /// Source RGB factor for blending
        GLenum sRGBFactor;

        /// Destination RGB factor for blending
        GLenum dRGBFactor;

        /// Source alpha factor for blending
        GLenum sAlphaFactor;

        /// Destination alpha factor for blendin
        GLenum dAlphaFactor;
    };
};

#endif // LCOLOR_H
