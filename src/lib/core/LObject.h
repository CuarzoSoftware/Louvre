#ifndef LOBJECT_H
#define LOBJECT_H

#include <LCompositor.h>

/**
 * @brief Base class for Louvre objects.
 */
class Louvre::LObject
{
public:
    /**
     * @brief Constructor of the LObject class.
     */
    inline LObject()
    {
        m_isAlive = new bool(true);
    }

    /**
     * @brief Destructor of the LObject class.
     */
    ~LObject();

    /**
     * @brief Quick access to the global compositor instance.
     */
    inline static LCompositor *compositor()
    {
        return LCompositor::compositor();
    }

    /**
     * @brief Quick access to the global seat instance.
     */
    inline static LSeat *seat()
    {
        return LCompositor::compositor()->seat();
    }

    /**
     * @brief Quick access to the global cursor instance.
     */
    inline static LCursor *cursor()
    {
        return LCompositor::compositor()->cursor();
    }

    /**
     * @brief Get a pointer to a boolean variable indicating the object's existence.
     *
     * The boolean variable is not immediately destroyed when the object is destructed,
     * making it safe to check its value even after the object is no longer present.
     * Developers may destroy an object during processing a request; therefore,
     * Louvre internally uses this property to verify its existence before accessing it.
     *
     * @return Pointer to the boolean variable indicating the object's existence.
     */
    inline const bool *isAlive() const
    {
        return m_isAlive;
    }

private:
    friend class LCompositor;
    bool *m_isAlive;
};

#endif // LOBJECT_H
