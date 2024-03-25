#ifndef LOBJECT_H
#define LOBJECT_H

#include <LNamespaces.h>
#include <vector>

/**
 * @brief Base class for Louvre objects.
 *
 * @see LWeak
 */
class Louvre::LObject
{
public:

    /**
     * @brief Copy constructor
     *
     * @note The user data and LWeak references are not copied.
     */
    LObject(const LObject &) noexcept {}

    /**
     * @brief Assignment operator (each object has its own individual LWeak reference count).
     *
     * @note The user data and LWeak references are not copied.
     */
    LObject &operator=(const LObject &) noexcept
    {
        return *this;
    }

    /**
     * @brief Store an unsigned integer value/pointer.
     */
    void setUserData(UIntPtr data) const noexcept
    {
        m_userData = data;
    }

    /**
     * @brief Retrieves the stored unsigned integer value/pointer.
     */
    UIntPtr userData() const noexcept
    {
        return m_userData;
    }

protected:

    /**
     * @brief Constructor of the LObject class.
     */
    LObject() noexcept = default;

    /**
     * @brief Destructor of the LObject class.
     */
    virtual ~LObject() noexcept;

    /**
     * @brief Notifies the object destruction.
     *
     * This method can be invoked from a subclass destructor to notify the object's imminent destruction
     * to all associated LWeak references in advance. If not invoked, the base LObject automatically calls it.
     *
     * After invocation, all LWeak references are set to `nullptr`, preventing the creation of additional references for this object.
     */
    void notifyDestruction() noexcept;

    /// @cond OMIT
private:
    friend class LWeakUtils;
    mutable std::vector<void*> m_weakRefs;
    mutable UIntPtr m_userData { 0 };
    bool m_destroyed { false };
    /// @endcond
};

#endif // LOBJECT_H
