#ifndef LWEAK_H
#define LWEAK_H

#include <LNamespaces.h>
#include <functional>

/// @cond OMIT
class Louvre::LWeakUtils
{
public:
    static std::vector<void *> &objectRefs(const LObject *object) noexcept;
    static bool isObjectDestroyed(const LObject *object) noexcept;
};
/// @endcond

/**
 * @brief Weak reference to an LObject pointer
 *
 * The LWeak class template provides a mechanism for creating weak pointer references to subclasses of LObject.
 * It is conceptually similar to `std::weak_ptr` but is specifically tailored for LObject subclasses, avoiding
 * the pointer indirection and associated performance overhead of the `std::weak_ptr` thread-safe mechanisms.
 *
 * When the object being referenced is destroyed, an optional on destroy callback event is emitted, and get() returns `nullptr`.
 */
template <class T>
class Louvre::LWeak
{
public:

    /**
     * Callback function type used to handle the `OnDestroy()` event.
     */
    using OnDestroyCallback = std::function<void(T*)>;

    /**
     * @brief Creates an empty LWeak
     */
    LWeak() noexcept = default;

    /**
     * @brief Creates a reference for the given LObject, or initializes an empty LWeak if `nullptr` is passed.
     *
     * @param object The LObject to create a reference for, or `nullptr` to initialize an empty LWeak.
     */
    LWeak(T *object) noexcept
    {
        static_assert(std::is_base_of<LObject, T>::value, "LWeak template error: Type must be a subclass of LObject.");

        if (object)
            pushBackTo(object);
    }

    /**
     * @brief Destructor, removes the LWeak from the LObject references.
     */
    ~LWeak() noexcept
    {
        clear();

        if (m_onDestroyCallback)
            delete m_onDestroyCallback;
    }

    /**
     * @brief Copy constructor, assigns the LObject of another LWeak.
     *
     * @param other The LWeak object to copy from.
     */
    LWeak(const LWeak<T> &other) noexcept
    {
        copy(other);
    }

    /**
     * @brief Assignment operator, assigns the LObject of another LWeak.
     *
     * @param other The LWeak object to assign from.
     * @return Reference to the updated LWeak object.
     */
    LWeak<T> &operator=(const LWeak<T> &other) noexcept
    {
        copy(other);
        return *this;
    }

    /**
     * @brief Get a pointer to the LObject or `nullptr` if not set or the object has been destroyed.
     *
     * @return Raw pointer to the referenced LObject.
     */
    T *get() const noexcept
    {
        return m_object;
    }

    /**
     * @brief Return the number of existing references to the current LObject.
     *
     * @return The number of existing references to the current LObject, if no object is set returns 0.
     */
    UInt64 count() const noexcept
    {
        if (m_object)
        {
            const auto &refs = (std::vector<LWeak<T>*>&)LWeakUtils::objectRefs((const LObject*)m_object);
            return refs.size();
        }

        return 0;
    }

    /**
     * @brief Replace the reference with another object.
     *
     * @param object The LObject to set as the new reference, or `nullptr` to unset the reference.
     */
    void reset(T *object = nullptr) noexcept
    {
        static_assert(std::is_base_of<LObject, T>::value, "LWeak template error: Type must be a subclass of LObject.");

        clear();

        if (object)
            pushBackTo(object);
    }

    /**
     * @brief Set the onDestroy callback function, pass `nullptr` to disable it.
     *
     * @note callback functions are not copied across LWeak instances.
     *
     * @param callback The callback function to be called when the referenced object is destroyed. Passing `nullptr` disables the callback.
     */
    void setOnDestroyCallback(const OnDestroyCallback &callback) noexcept
    {
        if (m_onDestroyCallback)
        {
            delete m_onDestroyCallback;
            m_onDestroyCallback = nullptr;
        }

        if (callback)
            m_onDestroyCallback = new OnDestroyCallback(callback);
    }

private:
    /// @cond OMIT
    friend class LObject;

    void copy(const LWeak<T> &other) noexcept
    {
        clear();

        if (other.m_object)
            pushBackTo(other.m_object);
    }

    void clear() noexcept
    {
        if (m_object)
        {
            auto &refs = (std::vector<LWeak<T>*>&)LWeakUtils::objectRefs((const LObject*)m_object);
            refs.back()->m_index = m_index;
            refs[m_index] = refs.back();
            refs.pop_back();
            m_object = nullptr;
        }
    }

    void pushBackTo(T *object) noexcept
    {
        if (LWeakUtils::isObjectDestroyed((const LObject*)object))
            return;

        m_object = object;
        auto &refs = (std::vector<LWeak<T>*>&)LWeakUtils::objectRefs((const LObject*)m_object);
        refs.push_back(this);
        m_index = refs.size() - 1;
    }

    T *m_object { nullptr };
    UInt64 m_index { 0 };
    OnDestroyCallback *m_onDestroyCallback { nullptr };
    /// @endcond OMIT
};

#endif // LWEAK_H
