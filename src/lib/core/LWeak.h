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
            auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(object);
            refs.push_back(this);
            m_object = object;
            return;
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
        return m_object;
    }

    inline UInt64 count() const
    {
        if (m_object)
        {
            const auto &refs = (std::vector<LWeak<T>*>&)PrivateUtils::getObjectData(m_object);
            return refs.size();
        }

        return 0;
    }

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

private:
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
};

#endif // LWEAK_H
