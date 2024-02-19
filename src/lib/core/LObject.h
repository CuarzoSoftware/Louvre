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
    inline LObject() {}

    /**
     * @brief Destructor of the LObject class.
     */
    inline ~LObject() {};

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
     * @brief Object's liveness status.
     *
     * This method returns a shared pointer to a constant boolean, allowing external
     * entities to monitor the liveness status of the object. The boolean's value is set to
     * `false` when the object is destroyed.
     *
     * @return A shared pointer to a constant boolean representing the liveness status.
     */
    inline std::shared_ptr<const bool> isAlive() const
    {
        return m_isAlive;
    }

private:
    std::shared_ptr<bool> m_isAlive { std::make_shared<bool>(true) };
};

#endif // LOBJECT_H
