#ifndef LBITFIELD_H
#define LBITFIELD_H

#include <LNamespaces.h>

template <class T>
class Louvre::LBitfield
{
    using Flag = typename std::conditional<
        std::is_enum<T>::value,
        typename std::underlying_type<T>::type,
        T
        >::type;

public:
    inline LBitfield(Flag flags = 0) : m_flags(flags) {}

    inline void add(Flag flags)
    {
        m_flags |= flags;
    }

    inline void remove(Flag flags)
    {
        m_flags &= ~flags;
    }

    inline bool check(Flag flags) const
    {
        return (m_flags & flags) != 0;
    }

    inline Flag get() const
    {
        return m_flags;
    }

    inline void set(Flag flags)
    {
        m_flags = flags;
    }

    inline void setFlag(Flag flag, bool enable)
    {
        if (enable)
            add(flag);
        else
            remove(flag);
    }

private:
    Flag m_flags;
};

#endif // LBITFIELD_H
