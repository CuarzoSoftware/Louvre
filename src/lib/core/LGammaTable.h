#ifndef LGAMMATABLE_H
#define LGAMMATABLE_H

#include <LNamespaces.h>
#include <string.h>

class Louvre::LGammaTable
{
public:
    inline LGammaTable(UInt32 size = 0)
    {
        setSize(size);
    }

    inline ~LGammaTable()
    {
        setSize(0);
    }

    inline LGammaTable(const LGammaTable &other)
    {
        setSize(other.size());
        if (size() > 0)
            memcpy(m_table, other.m_table, size() * 3 * sizeof(UInt16));
    }

    LGammaTable &operator=(const LGammaTable &other)
    {
        setSize(other.size());
        if (size() > 0)
            memcpy(m_table, other.m_table, size() * 3 * sizeof(UInt16));
        return *this;
    }

    inline void setSize(UInt32 size)
    {
        if (size != m_size)
        {
            m_size = size;

            if (m_table)
            {
                delete[] m_table;
                m_table = nullptr;
            }

            if (size > 0)
                m_table = new UInt16[size * 3];
        }
    }

    inline UInt32 size() const
    {
        return m_size;
    }

    void fill(Float64 gamma, Float64 brightness, Float64 contrast);

    inline UInt16 *red() const
    {
        return m_table;
    }

    inline UInt16 *green() const
    {
        if (m_size == 0)
            return nullptr;

        return m_table + m_size;
    }

    inline UInt16 *blue() const
    {
        if (m_size == 0)
            return nullptr;

        return m_table + m_size + m_size;
    }
private:
    UInt32 m_size = 0;
    UInt16 *m_table = nullptr;
};

#endif // LGAMMATABLE_H
