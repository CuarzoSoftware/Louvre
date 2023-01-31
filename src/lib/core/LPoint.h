#ifndef LPOINT_H
#define LPOINT_H

#include <LNamespaces.h>

using namespace Louvre;

/*!
 * @brief Template for 2D vectors
 *
 * La clase LPointTemplate es un template para crear vectores de dos dimensiones.\n
 * La librería utiliza este template para generar las clases Louvre::LPoint y Louvre::LSize las cuales trabajan con
 * enteros de 32 bits y Louvre::LPointF y Louvre::LSizeF que trabajan con flotantes de 32 bits.
 *
 */
template <class TA, class TB>
class Louvre::LPointTemplate
{
public:

    /// Inicializa el vector con (0,0).
    inline LPointTemplate() : m_x(0), m_y(0) {}

    /// Inicializa el vector con (x,y).
    inline LPointTemplate(TA x, TA y) : m_x(x), m_y(y){}

    /// Inicializa el vector con (x,y).
    inline LPointTemplate(TB x, TB y) : m_x(x), m_y(y){}

    /// Inicializa el vector con (n,n).
    inline LPointTemplate(TA n) : m_x(n), m_y(n){}

    /// Inicializa el vector con (n,n)
    inline LPointTemplate(TB n) : m_x(n), m_y(n){}

    /// Constructor de copia
    inline LPointTemplate(const LPointTemplate<TB,TA> &point)
    {
        m_x = point.m_x;
        m_y = point.m_y;
    }

    /// Primera componente del vector
    inline TA x()       const   {return m_x;}

    /// Segunda componente del vector
    inline TA y()       const   {return m_y;}

    /// Primera componente del vector (equivalente a x())
    inline TA w()       const   {return m_x;}

    /// Segunda componente del vector (equivalente a y())
    inline TA h()       const   {return m_y;}

    /// Primera componente del vector (equivalente a x())
    inline TA width()   const   {return m_x;}

    /// Segunda componente del vector (equivalente a y())
    inline TA height()  const   {return m_y;}

    /// Multiplicación de las componentes (x*y)
    inline TA area()    const   {return m_x*m_y;}

    /// Asigna la primera componente del vector
    inline void setX(TA x){m_x = x;}

    /// Asigna la segunda componente del vector
    inline void setY(TA y){m_y = y;}

    /// Asigna la primera componente del vector (equivalente a setX())
    inline void setW(TA x){m_x = x;}

    /// Asigna la segunda componente del vector (equivalente a setY())
    inline void setH(TA y){m_y = y;}

    /// Asigna la primera componente del vector (equivalente a setX())
    inline void setWidth(TA x){m_x = x;}

    /// Asigna la segunda componente del vector (equivalente a setY())
    inline void setHeight(TA y){m_y = y;}

    /// Asigna la segunda componente manteniendo la proporción con la primera
    inline LPointTemplate<TA,TB> constrainedToHeight(TA size) const
    {
        LPointTemplate point;

        if(size == 0 || h() == 0)
            return point;

        point.setW((w()*size)/h());
        point.setH(size);

        return point;
    }

    inline LPointTemplate<TA,TB> &operator+=(TA factor)
    {
        m_x += factor;
        m_y += factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator-=(TA factor)
    {
        m_x -= factor;
        m_y -= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator*=(TA factor)
    {
        m_x *= factor;
        m_y *= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator/=(TA factor)
    {
        m_x /= factor;
        m_y /= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator+=(TB factor)
    {
        m_x += factor;
        m_y += factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator-=(TB factor)
    {
        m_x -= factor;
        m_y -= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator*=(TB factor)
    {
        m_x *= factor;
        m_y *= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator/=(TB factor)
    {
        m_x /= factor;
        m_y /= factor;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator+=(const LPointTemplate<TA,TB> &p)
    {
        m_x += p.m_x;
        m_y += p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator-=(const LPointTemplate<TA,TB> &p)
    {
        m_x -= p.m_x;
        m_y -= p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator*=(const LPointTemplate<TA,TB> &p)
    {
        m_x *= p.m_x;
        m_y *= p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator/=(const LPointTemplate<TA,TB> &p)
    {
        m_x /= p.m_x;
        m_y /= p.m_y;
        return *this;
    }

    /***************************************************************/

    inline LPointTemplate<TA,TB> &operator+=(const LPointTemplate<TB,TA> &p)
    {
        m_x += p.m_x;
        m_y += p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator-=(const LPointTemplate<TB,TA> &p)
    {
        m_x -= p.m_x;
        m_y -= p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator*=(const LPointTemplate<TB,TA> &p)
    {
        m_x *= p.m_x;
        m_y *= p.m_y;
        return *this;
    }

    inline LPointTemplate<TA,TB> &operator/=(const LPointTemplate<TB,TA> &p)
    {
        m_x /= p.m_x;
        m_y /= p.m_y;
        return *this;
    }

    /***************************************************************/

    inline LPointTemplate<TA,TB> operator+(TA factor) const
    {
        return LPointTemplate<TA,TB>(m_x+factor,m_y+factor);
    }

    inline LPointTemplate<TA,TB> operator-(TA factor) const
    {
        return LPointTemplate<TA,TB>(m_x-factor,m_y-factor);
    }

    inline LPointTemplate<TA,TB> operator*(TA factor) const
    {
        return LPointTemplate<TA,TB>(m_x*factor,m_y*factor);
    }

    inline LPointTemplate<TA,TB> operator/(TA factor) const
    {
        return LPointTemplate<TA,TB>(m_x/factor,m_y/factor);
    }

    inline LPointTemplate<TA,TB> operator+(TB factor) const
    {
        return LPointTemplate<TA,TB>(m_x+factor,m_y+factor);
    }

    inline LPointTemplate<TA,TB> operator-(TB factor) const
    {
        return LPointTemplate<TA,TB>(m_x-factor,m_y-factor);
    }

    inline LPointTemplate<TA,TB> operator*(TB factor) const
    {
        return LPointTemplate<TA,TB>(m_x*factor,m_y*factor);
    }

    inline LPointTemplate<TA,TB> operator/(TB factor) const
    {
        return LPointTemplate<TA,TB>(m_x/factor,m_y/factor);
    }

    inline LPointTemplate<TA,TB> operator+(const LPointTemplate<TA,TB> &p) const
    {
        return LPointTemplate<TA,TB>(m_x+p.m_x,m_y+p.m_y);
    }

    inline LPointTemplate<TA,TB> operator-(const LPointTemplate<TA,TB> &p) const
    {
        return LPointTemplate<TA,TB>(m_x-p.m_x,m_y-p.m_y);
    }

    inline LPointTemplate<TA,TB> operator*(const LPointTemplate<TA,TB> &p) const
    {
        return LPointTemplate<TA,TB>(m_x*p.m_x,m_y*p.m_y);
    }

    inline LPointTemplate<TA,TB> operator/(const LPointTemplate<TA,TB> &p) const
    {
        return LPointTemplate<TA,TB>(m_x/p.m_x,m_y/p.m_y);
    }

    /***************************************************************/

    inline LPointTemplate<TA,TB> operator+(const LPointTemplate<TB,TA> &p) const
    {
        return LPointTemplate(m_x+p.m_x,m_y+p.m_y);
    }

    inline LPointTemplate<TA,TB> operator-(const LPointTemplate<TB,TA> &p) const
    {
        return LPointTemplate(m_x-p.m_x,m_y-p.m_y);
    }

    inline LPointTemplate<TA,TB> operator*(const LPointTemplate<TB,TA> &p) const
    {
        return LPointTemplate(m_x*p.m_x,m_y*p.m_y);
    }

    inline LPointTemplate<TA,TB> operator/(const LPointTemplate<TB,TA> &p) const
    {
        return LPointTemplate(m_x/p.m_x,m_y/p.m_y);
    }

    /***************************************************************/

    inline bool operator==(const LPointTemplate<TA,TB> &p) const
    {
        return m_x == p.m_x && m_y == p.m_y;
    }

    inline bool operator!=(const LPointTemplate<TA,TB> &p) const
    {
        return m_x != p.m_x || m_y != p.m_y;
    }

private:
    friend class LRectTemplate<TA,TB>;
    friend class LPointTemplate<TB,TA>;
    TA m_x = 0;
    TA m_y = 0;
};

inline LPoint operator+(Int32 i, LPoint p)
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

inline LPoint operator-(Int32 i, LPoint p)
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

inline LPoint operator*(Int32 i, LPoint p)
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

inline LPointF operator+(Int32 i, LPointF p)
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

inline LPointF operator-(Int32 i, LPointF p)
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

inline LPointF operator*(Int32 i, LPointF p)
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

#endif // LPOINT_H
