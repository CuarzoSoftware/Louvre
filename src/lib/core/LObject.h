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

    inline std::weak_ptr<const LObject> weakRef() const
    {
        return m_sharedPtr;
    }

    inline std::weak_ptr<LObject> weakRef()
    {
        return m_sharedPtr;
    }

    template <class T>
    inline std::weak_ptr<const T> weakRef() const
    {
        return std::static_pointer_cast<const T>(m_sharedPtr);
    }

    template <class T>
    inline std::weak_ptr<T> weakRef()
    {
        return std::static_pointer_cast<T>(m_sharedPtr);
    }

protected:

    /**
     * @brief Constructor of the LObject class.
     */
    inline LObject() = default;

    /**
     * @brief Destructor of the LObject class.
     */
    inline ~LObject() = default;
private:
    std::shared_ptr<LObject> m_sharedPtr { this, [](auto){}};
};

#endif // LOBJECT_H
