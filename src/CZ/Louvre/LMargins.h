#ifndef LMARGINS_H
#define LMARGINS_H

#include <CZ/Louvre/LNamespaces.h>

namespace Louvre
{
    /**
     * @brief Structure representing left, top, right and bottom margins.
     */
    struct LMargins
    {
        Int32 left {0}; ///< The left margin.
        Int32 top {0}; ///< The top margin.
        Int32 right {0}; ///< The right margin.
        Int32 bottom {0}; ///< The bottom margin.
    };
};

#endif // LMARGINS_H
