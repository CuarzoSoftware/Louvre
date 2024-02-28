#ifndef LWEAK_H
#define LWEAK_H

#include <LNamespaces.h>

// Todo add doc

template <class T>
class Louvre::LWeak
{
    static_assert(std::is_base_of<LObject, T>::value, "LWeak template error: Type must be a subclass of LObject.");

public:
    inline LWeak(T *object = nullptr)
    {
        if (object)
        {
            const LObject *obj { object };
            m_data = PrivateUtils::getObjectData(obj);
            m_data->counter++;
            m_object = object;
        }
    }

    inline ~LWeak()
    {
        clear();
    }

    inline LWeak(const LWeak &other)
    {
        copy(other);
    }

    inline LWeak &operator=(const LWeak &other)
    {
        copy(other);
        return *this;
    }

    inline T *get() const
    {
        if (m_data && m_data->isAlive)
            return m_object;
        return nullptr;
    }

    inline UInt32 count() const
    {
        if (m_data)
            return m_data->counter;

        return 0;
    }

    void reset(T *object  = nullptr)
    {
        clear();

        if (object)
        {
            const LObject *obj { object };
            m_data = PrivateUtils::getObjectData(obj);
            m_data->counter++;
            m_object = object;
        }
    }

private:
    friend class LObject;

    inline void copy(const LWeak &other)
    {
        clear();

        if (other.m_data)
        {
            m_data = other.m_data;
            m_object = other.m_object;
            m_data->counter++;
        }
    }

    inline void clear()
    {
        if (m_data)
        {
            if (m_data->counter == 1)
                delete m_data;
            else
                m_data->counter--;

            m_data = nullptr;
        }
    }

    LWeakData *m_data { nullptr };
    T *m_object;
};

#endif // LWEAK_H
