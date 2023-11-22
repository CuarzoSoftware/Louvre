#ifndef LOBJECT_H
#define LOBJECT_H

#include <LNamespaces.h>

/**
 * @brief Base class for Louvre objects.
 */
class Louvre::LObject
{
public:
    /**
     * @brief Constructor of the LObject class.
     */
    LObject();

    /**
     * @brief Quick access to the global compositor instance.
     */
    static LCompositor *compositor();

    /**
     * @brief Quick access to the global seat instance.
     */
    static LSeat *seat();

    /**
     * @brief Quick access to the global cursor instance.
     */
    static LCursor *cursor();

    /// @cond OMIT
    // LPRIVATE_IMP(LObject); (not required yet)
    /// @endcond
};

#endif // LOBJECT_H
