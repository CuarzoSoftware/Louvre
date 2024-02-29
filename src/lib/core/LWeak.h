#ifndef LWEAK_H
#define LWEAK_H

#include <LNamespaces.h>
#include <functional>

/**
 * @brief Weak reference to an LObject pointer
 *
 * The LWeak class template provides a mechanism for creating weak references to subclasses of LObject.
 * It is conceptually similar to `std::weak_ptr` but is specifically tailored for LObject subclasses, avoiding
 * the pointer indirection and associated performance overhead of the `std::weak_ptr` thread-safe mechanisms.
 *
 * When the object being referenced is destroyed, an optional on destroy callback event is emitted, and get() returns `nullptr`.
 *
 * @see LObject::weakRef()
 */
template <class T>
class Louvre::LWeak
{
    static_assert(std::is_base_of<LObject, T>::value, "LWeak template error: Type must be a subclass of LObject.");

public:

    /**
     * Callback function type used to handle the `OnDestroy()` event.
     */
    using Callback = std::function<void(LWeak<T>*)>;

    /**
     * @brief Creates a reference for the given LObject, or initializes an empty LWeak if `nullptr` is passed.
     *
     * @param object The LObject to create a reference for, or `nullptr` to initialize an empty LWeak.
     */
    inline LWeak(T *object = nullptr)
    {
        if (object)
        {
            auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(object);
            refs.push_back(this);
            m_object = object;
            return;
        }
    }

    /**
     * @brief Destructor, removes the LWeak from the LObject references.
     */
    inline ~LWeak()
    {
        clear();
    }

    /**
     * @brief Copy constructor, assigns the LObject of another LWeak.
     *
     * @param other The LWeak object to copy from.
     */
    inline LWeak(const LWeak &other)
    {
        copy(other);
    }

    /**
     * @brief Assignment operator, assigns the LObject of another LWeak.
     *
     * @param other The LWeak object to assign from.
     * @return Reference to the updated LWeak object.
     */
    inline LWeak &operator=(const LWeak &other)
    {
        copy(other);
        return *this;
    }

    /**
     * @brief Get a pointer to the LObject or `nullptr` if not set or the object has been destroyed.
     *
     * @return Raw pointer to the referenced LObject.
     */
    inline T *get() const
    {
        return m_object;
    }

    /**
     * @brief Return the number of existing references to the current LObject.
     *
     * @return The number of existing references to the current LObject, if no object is set returns 0.
     */
    inline UInt64 count() const
    {
        if (m_object)
        {
            const auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(m_object);
            return refs.size();
        }

        return 0;
    }

    /**
     * @brief Replace the reference with another object.
     *
     * @param object The LObject to set as the new reference, or `nullptr` to unset the reference.
     */
    void reset(T *object = nullptr)
    {
        clear();

        if (object)
        {
            auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(object);
            refs.push_back(this);
            m_object = object;
            return;
        }
    }

    /**
     * @brief Set the onDestroy callback function, pass `nullptr` to disable it.
     *
     * @param callback The callback function to be called when the referenced object is destroyed.
     */
    void setOnDestroyCallback(Callback callback)
    {
        m_onDestroyCallback = callback;
    }

private:
    /// @cond OMIT
    friend class LObject;

    inline void copy(const LWeak &other)
    {
        clear();

        if (other.m_object)
        {
            auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(other.m_object);
            refs.push_back(this);
            m_object = other.m_object;
            return;
        }
    }

    inline void clear()
    {
        if (m_object)
        {
            auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(m_object);
            LVectorRemoveOneUnordered(refs, this);
            m_object = nullptr;
        }
    }

    T *m_object { nullptr };
    Callback m_onDestroyCallback { nullptr };
    /// @endcond OMIT
};

#endif // LWEAK_H
