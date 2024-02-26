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
    inline ~LObject()
    {
        if (m_weakData->counter == 1)
            delete m_weakData;
        else
        {
            m_weakData->counter--;
            m_weakData->isAlive = false;
        }
    }
private:
    friend class Louvre::PrivateUtils;
    LWeakData *m_weakData { new LWeakData({1, true}) };
};

#endif // LOBJECT_H
