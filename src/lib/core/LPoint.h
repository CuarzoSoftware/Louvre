#ifndef LPOINT_H
#define LPOINT_H

#include <LNamespaces.h>

/**
 * @brief Template for 2D vectors
 *
 * The LPointTemplate class is a template for creating two-dimensional vectors.\n
 * The library uses this template to generate the LPoint and LSize classes, which work with
 * 32-bit integers, and LPointF and LSizeF, which work with 32-bit floating-point numbers.
 */
template <class TA, class TB>
class Louvre::LPointTemplate
{
public:

    /// Initializes the vector with (0,0).
    inline LPointTemplate() : m_x(0), m_y(0) {}

    /// Initializes the vector with (x,y).
    inline LPointTemplate(TA x, TA y) : m_x(x), m_y(y) {}

    /// Initializes the vector with (x,y).
    inline LPointTemplate(TB x, TB y) : m_x(x), m_y(y) {}

    /// Initializes the vector with (n,n).
    inline LPointTemplate(TA n) : m_x(n), m_y(n) {}

    /// Initializes the vector with (n,n).
    inline LPointTemplate(TB n) : m_x(n), m_y(n) {}

    /// Copy constructor
    inline LPointTemplate(const LPointTemplate<TB,TA> &point)
    {
        m_x = point.m_x;
        m_y = point.m_y;
    }

    /// First component of the vector
    inline TA x() const { return m_x; }

    /// Second component of the vector
    inline TA y() const { return m_y; }

    /// First component of the vector (equivalent to x())
    inline TA w() const { return m_x; }

    /// Second component of the vector (equivalent to y())
    inline TA h() const { return m_y; }

    /// First component of the vector (equivalent to x())
    inline TA width() const { return m_x; }

    /// Second component of the vector (equivalent to y())
    inline TA height() const { return m_y; }

    /// Product of the components (x*y)
    inline TA area() const { return m_x * m_y; }

    /// Assigns the value to the first component of the vector
    inline void setX(TA x) { m_x = x; }

    /// Assigns the value to the second component of the vector
    inline void setY(TA y) { m_y = y; }

    /// Assigns the value to the first component of the vector (equivalent to setX())
    inline void setW(TA x) { m_x = x; }

    /// Assigns the value to the second component of the vector (equivalent to setY())
    inline void setH(TA y) { m_y = y; }

    /// Assigns the value to the first component of the vector (equivalent to setX())
    inline void setWidth(TA x) { m_x = x; }

    /// Assigns the value to the second component of the vector (equivalent to setY())
    inline void setHeight(TA y) { m_y = y; }

    /// Assigns the second component while maintaining proportion with the first
    inline LPointTemplate<TA, TB> constrainedToHeight(TA size) const
    {
        LPointTemplate point;

        if (size == 0 || h() == 0)
            return point;

        point.setW((w() * size) / h());
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

    inline bool operator==(const LPointTemplate<TB,TA> &p) const
    {
        return m_x == p.m_x && m_y == p.m_y;
    }

    inline bool operator!=(const LPointTemplate<TA,TB> &p) const
    {
        return m_x != p.m_x || m_y != p.m_y;
    }

    inline bool operator!=(const LPointTemplate<TB,TA> &p) const
    {
        return m_x != p.m_x || m_y != p.m_y;
    }

private:
    friend class LRectTemplate<TA,TB>;
    friend class LPointTemplate<TB,TA>;
    TA m_x = 0;
    TA m_y = 0;
};

inline Louvre::LPoint operator+(Louvre::Int32 i, Louvre::LPoint p)
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

inline Louvre::LPoint operator-(Louvre::Int32 i, Louvre::LPoint p)
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

inline Louvre::LPoint operator*(Louvre::Int32 i, Louvre::LPoint p)
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

inline Louvre::LPointF operator+(Louvre::Int32 i, Louvre::LPointF p)
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

inline Louvre::LPointF operator-(Louvre::Int32 i, Louvre::LPointF p)
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

inline Louvre::LPointF operator*(Louvre::Int32 i, Louvre::LPointF p)
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

#endif // LPOINT_H
