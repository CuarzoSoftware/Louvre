#ifndef LOBJECT_H
#define LOBJECT_H

#include <LCompositor.h>
#include <LWeak.h>

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

    inline LObject(const LObject &) {}

    inline LObject &operator=(const LObject &)
    {
        return *this;
    }

    inline LWeak<const LObject> weakRef() const
    {
        return LWeak<const LObject>(this);
    }

    inline LWeak<LObject> weakRef()
    {
        return LWeak<LObject>(this);
    }

    template <class T>
    inline LWeak<const T> weakRef() const
    {
        return LWeak<const T>((const T*)this);
    }

    template <class T>
    inline LWeak<T> weakRef()
    {
        return LWeak<T>((T*)this);
    }

protected:

    /**
     * @brief Constructor of the LObject class.
     */
    inline LObject() = default;

    /**
     * @brief Destructor of the LObject class.
     */
    virtual ~LObject()
    {
        while (!m_weakRefs.empty())
        {
            LWeak<LObject> *weak = (LWeak<LObject>*)m_weakRefs.back();
            m_weakRefs.pop_back();

            if (weak->m_onDestroyCallback)
                weak->m_onDestroyCallback(weak);

            if (weak->m_object == this)
                weak->m_object = nullptr;
        }
    }
private:
    friend class Louvre::PrivateUtils;
    mutable std::vector<void*> m_weakRefs;
};

#endif // LOBJECT_H
