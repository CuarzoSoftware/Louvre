#ifndef LOBJECT_H
#define LOBJECT_H

#include <LCompositor.h>
#include <LWeak.h>
#include <cassert>

/**
 * @brief Base class for Louvre objects.
 *
 * LObject is the base class of many Louvre classes. It can be used in conjunction with LWeak to
 * create weak pointer references, which are automatically set to `nullptr` when the object is destroyed.
 * You can also set a custom callback upon destruction, see LWeak::setOnDestroyCallback().
 *
 * To notify destruction in advance, you can call notifyDestruction() from the subclass destructor.
 * After calling it, no more references can be made from the object.
 */
class Louvre::LObject
{
public:

    /**
     * @brief Quick access to the global compositor instance.
     */
    inline static LCompositor *compositor() noexcept
    {
        return LCompositor::compositor();
    }

    /**
     * @brief Quick access to the global seat instance.
     */
    inline static LSeat *seat() noexcept
    {
        return LCompositor::compositor()->seat();
    }

    /**
     * @brief Quick access to the global cursor instance.
     */
    inline static LCursor *cursor() noexcept
    {
        return LCompositor::compositor()->cursor();
    }

    /**
     * @brief Copy constructor (each object has its own individual LWeak reference count).
     */
    inline LObject(const LObject &) noexcept {}

    /**
     * @brief Assignment operator (each object has its own individual LWeak reference count).
     */
    inline LObject &operator=(const LObject &) noexcept
    {
        return *this;
    }

    /**
     * @brief Return an LWeak reference of type const LObject.
     */
    inline LWeak<const LObject> weakRef() const noexcept
    {
        return LWeak<const LObject>(this);
    }

    /**
     * @brief Return an LWeak reference of type LObject.
     */
    inline LWeak<LObject> weakRef() noexcept
    {
        return LWeak<LObject>(this);
    }

    /**
     * @brief Return an LWeak reference of type const T.
     *
     * @tparam T The type to create an LWeak reference for.
     */
    template <class T>
    inline LWeak<const T> weakRef() const noexcept
    {
        return LWeak<const T>(static_cast<const T*>(this));
    }

    /**
     * @brief Return an LWeak reference of type T.
     *
     * @tparam T The type to create an LWeak reference for.
     */
    template <class T>
    inline LWeak<T> weakRef() noexcept
    {
        return LWeak<T>(static_cast<T*>(this));
    }

    UInt64 id { 0 };

protected:

    /**
     * @brief Constructor of the LObject class.
     */
    inline LObject() noexcept = default;

    /**
     * @brief Destructor of the LObject class.
     */
    virtual ~LObject() noexcept
    {
        notifyDestruction();
    }

    /**
     * @brief Notifies object destruction to references and sets them to `nullptr`.
     *
     * This method can be invoked from a subclass destructor to notify the object's imminent destruction
     * to all associated LWeak references in advance. If not invoked, the base LObject automatically calls it.
     *
     * After invocation, all references are set to `nullptr`, preventing the creation of additional references for this object.
     */
    inline void notifyDestruction() noexcept
    {
        if (m_destroyed)
            return;

        m_destroyed = true;

        while (!m_weakRefs.empty())
        {
            LWeak<LObject> *weak { (LWeak<LObject>*)m_weakRefs.back() };
            weak->m_object = nullptr;
            m_weakRefs.pop_back();

            if (weak->m_onDestroyCallback)
                (*weak->m_onDestroyCallback)(this);
        }
    }
private:
    friend class Louvre::PrivateUtils;
    mutable std::vector<void*> m_weakRefs;
    bool m_destroyed { false };
};

#endif // LOBJECT_H
