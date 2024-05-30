#ifndef LEDGE_H
#define LEDGE_H

#include <LBitset.h>

namespace Louvre
{
    /**
     * @brief Edge flags.
     */
    enum LEdge : UInt32
    {
        LEdgeNone    = 0, ///< No edge.
        LEdgeTop     = static_cast<UInt32>(1) << 0, ///< The top edge.
        LEdgeBottom  = static_cast<UInt32>(1) << 1, ///< The bottom edge.
        LEdgeLeft    = static_cast<UInt32>(1) << 2, ///< The left edge.
        LEdgeRight   = static_cast<UInt32>(1) << 3, ///< The right edge.
    };

    /**
     * @brief Checks if the given edges form a corner by being orthogonal.
     *
     * This function verifies if the provided bitset of edges corresponds to one of the four possible
     * corner configurations: (Top-Left, Top-Right, Bottom-Left, Bottom-Right).
     *
     * @param edges A bitset representing the edges.
     * @return `true` if the edges form a corner, `false` otherwise.
     */
    inline constexpr bool edgeIsCorner(LBitset<LEdge> edges) noexcept
    {
        return edges == (LEdgeTop | LEdgeLeft)
            || edges == (LEdgeTop | LEdgeRight)
            || edges == (LEdgeBottom | LEdgeLeft)
            || edges == (LEdgeBottom | LEdgeRight);
    }
};

#endif // LEDGE_H
