#ifndef LRECT_H
#define LRECT_H

#include <LNamespaces.h>
#include <LPoint.h>

/**
 * @brief Template for 4D vectors
 *
 * The LRectTemplate class is a template for creating four-dimensional vectors.\n
 * The library uses this template to generate the classes LRect which works with
 * 32-bit integers and LRectF which works with 32-bit floats.
*/
template <class TA, class TB>
class Louvre::LRectTemplate
{
public:

    /// Initializes the vector with (0,0,0,0).
    inline constexpr LRectTemplate() noexcept {}

    /// Initializes the vector with (x,y,width,height).
    inline constexpr LRectTemplate(TA x, TA y, TA width, TA height) noexcept
    {
        m_topLeft.m_x       = x;
        m_topLeft.m_y       = y;
        m_bottomRight.m_x   = width;
        m_bottomRight.m_y   = height;
    }

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    inline constexpr LRectTemplate(const LPointTemplate<TA,TB> &topLeft, const LPointTemplate<TA,TB> &bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    inline constexpr LRectTemplate(TA topLeft, const LPointTemplate<TA,TB> &bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    inline constexpr LRectTemplate(const LPointTemplate<TA,TB> &topLeft, TA bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (all,all,all,all).
    inline constexpr LRectTemplate(TA all) noexcept : m_topLeft(all), m_bottomRight(all){}

    /// Initializes the vector with (p.x(),p.y(),p.x(),p.y()).
    inline constexpr LRectTemplate(const LPointTemplate<TA,TB> &p) noexcept : m_topLeft(p), m_bottomRight(p){}

    /// Copy constructor
    inline constexpr LRectTemplate(const LRectTemplate<TB,TA> &r) noexcept
    {
        m_topLeft = r.m_topLeft;
        m_bottomRight = r.m_bottomRight;
    }

    /// First vector component
    inline constexpr TA x()       const noexcept   {return m_topLeft.m_x;}

    /// Second vector component
    inline constexpr TA y()       const noexcept   {return m_topLeft.m_y;}

    /// Third vector component
    inline constexpr TA w()       const noexcept   {return m_bottomRight.m_x;}

    /// Fourth vector component
    inline constexpr TA h()       const noexcept   {return m_bottomRight.m_y;}

    /// Third vector component
    inline constexpr TA width()   const noexcept   {return m_bottomRight.m_x;}

    /// Fourth vector component
    inline constexpr TA height()  const noexcept   {return m_bottomRight.m_y;}

    /// The multiplication of the third and fourth component (width*height)
    inline constexpr TA area()    const noexcept   {return m_bottomRight.m_x*m_bottomRight.m_y;}

    /**
     * Returns true if the rectangle contains the point.
     *
     * @param point The 2D vector to check
     * @param inclusive If true, the edges of the rectangle are considered as part of the rectangle.
     */
    inline constexpr bool containsPoint(const LPointTemplate<TA,TB> &point, bool inclusive = true) const noexcept
    {
        if(inclusive)
        {
            return point.m_x >= m_topLeft.m_x &&
                   point.m_x <= m_topLeft.m_x + m_bottomRight.m_x &&
                   point.m_y >= m_topLeft.m_y &&
                   point.m_y <= m_topLeft.m_y + m_bottomRight.m_y;
        }

        return point.m_x > m_topLeft.m_x &&
               point.m_x < m_topLeft.m_x + m_bottomRight.m_x &&
               point.m_y > m_topLeft.m_y &&
               point.m_y < m_topLeft.m_y + m_bottomRight.m_y;
    }

    /**
     * Returns true if the rectangles intersect.
     *
     * @param rect Rectangle to intersect
     * @param inclusive If true, the edges of the rectangle are considered
     */
    inline constexpr bool intersects(const LRectTemplate &rect, bool inclusive = true) const noexcept
    {
        if(inclusive)
        {
            return x() <= rect.x()+rect.w() &&
                   x() + w() >= rect.x() &&
                   y() <= rect.y()+rect.h() &&
                   y()+h() >= rect.y();
        }

        return x() < rect.x()+rect.w() &&
               x() + w() > rect.x() &&
               y() < rect.y()+rect.h() &&
               y()+h() > rect.y();
    }

    /// 2D vector given by the (x,y) components of the rectangle
    inline constexpr const LPointTemplate<TA,TB> &topLeft() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    inline constexpr const LSize &bottomRight() const noexcept {return m_bottomRight;}

    /// 2D vector given by the (x,y) components of the rectangle
    inline constexpr const LPointTemplate<TA,TB> &TL() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    inline constexpr const LSize &BR() const noexcept {return m_bottomRight;}

    /// 2D vector given by the (x,y) components of the rectangle
    inline constexpr const LPointTemplate<TA,TB> &pos() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    inline constexpr const LSize &size() const noexcept {return m_bottomRight;}

    /// Asigns the first component
    inline constexpr void setX(TA x) noexcept {m_topLeft.m_x = x;}

    /// Asigns the second component
    inline constexpr void setY(TA y) noexcept {m_topLeft.m_y = y;}

    /// Asigns the third component
    inline constexpr void setW(TA width) noexcept {m_bottomRight.m_x = width;}

    /// Asigns the fourth component
    inline constexpr void setH(TA height) noexcept {m_bottomRight.m_y = height;}

    /// Asigns the third component
    inline constexpr void setWidth(TA width) noexcept {m_bottomRight.m_x = width;}

    /// Asigns the fourth component
    inline constexpr void setHeight(TA height) noexcept {m_bottomRight.m_y = height;}

    /// Asigns the (x,y) components
    inline constexpr void setTL(const LPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setBR(const LPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    inline constexpr void setTL(const LPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setBR(const LPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    inline constexpr void setTopLeft(const LPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setBottomRight(const LPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    inline constexpr void setTopLeft(const LPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setBottomRight(const LPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    inline constexpr void setPos(const LPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setSize(const LPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    inline constexpr void setPos(const LPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    inline constexpr void setSize(const LPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Returns true if the resulting rectangle has an area of 0.
    inline constexpr bool clip(const LRectTemplate &rect) noexcept
    {
        TA x0 = x();
        TA x1 = x0 + w();
        TA y0 = y();
        TA y1 = y0 + h();

        TA rx1 = rect.x() + rect.w();
        TA ry1 = rect.y() + rect.h();

        // X
        if(rect.x() > x0)
            x0 = rect.x();
        else if(rx1 < x0)
            x0 = rx1;

        // W
        if(rect.x() > x1)
            x1 = rect.x();
        else if(rx1 < x1)
            x1 = rx1;

        // Y
        if(rect.y() > y0)
            y0 = rect.y();
        else if(ry1 < y0)
            y0 = ry1;

        // H
        if(rect.y() > y1)
            y1 = rect.y();
        else if(ry1 < y1)
            y1 = ry1;

        setX(x0);
        setY(y0);
        setW(x1 - x0);
        setH(y1 - y0);

        return (w() == 0 || h() == 0 );
    }

    inline constexpr LRectTemplate &operator+=(TA factor) noexcept
    {
        m_topLeft.m_x += factor;
        m_topLeft.m_y += factor;
        m_bottomRight.m_x += factor;
        m_bottomRight.m_y += factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator-=(TA factor) noexcept
    {
        m_topLeft.m_x -= factor;
        m_topLeft.m_y -= factor;
        m_bottomRight.m_x -= factor;
        m_bottomRight.m_y -= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator*=(TA factor) noexcept
    {
        m_topLeft.m_x *= factor;
        m_topLeft.m_y *= factor;
        m_bottomRight.m_x *= factor;
        m_bottomRight.m_y *= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator/=(TA factor) noexcept
    {
        m_topLeft.m_x /= factor;
        m_topLeft.m_y /= factor;
        m_bottomRight.m_x /= factor;
        m_bottomRight.m_y /= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator+=(TB factor) noexcept
    {
        m_topLeft.m_x += factor;
        m_topLeft.m_y += factor;
        m_bottomRight.m_x += factor;
        m_bottomRight.m_y += factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator-=(TB factor) noexcept
    {
        m_topLeft.m_x -= factor;
        m_topLeft.m_y -= factor;
        m_bottomRight.m_x -= factor;
        m_bottomRight.m_y -= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator*=(TB factor) noexcept
    {
        m_topLeft.m_x *= factor;
        m_topLeft.m_y *= factor;
        m_bottomRight.m_x *= factor;
        m_bottomRight.m_y *= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator/=(TB factor) noexcept
    {
        m_topLeft.m_x /= factor;
        m_topLeft.m_y /= factor;
        m_bottomRight.m_x /= factor;
        m_bottomRight.m_y /= factor;
        return *this;
    }

    inline constexpr LRectTemplate &operator+=(const LRectTemplate &r) noexcept
    {
        m_topLeft.m_x += r.m_topLeft.m_x;
        m_topLeft.m_y += r.m_topLeft.m_y;
        m_bottomRight.m_x += r.m_bottomRight.m_x;
        m_bottomRight.m_y += r.m_bottomRight.m_y;
        return *this;
    }

    inline constexpr LRectTemplate &operator-=(const LRectTemplate &r) noexcept
    {
        m_topLeft.m_x -= r.m_topLeft.m_x;
        m_topLeft.m_y -= r.m_topLeft.m_y;
        m_bottomRight.m_x -= r.m_bottomRight.m_x;
        m_bottomRight.m_y -= r.m_bottomRight.m_y;
        return *this;
    }

    inline constexpr LRectTemplate &operator*=(const LRectTemplate &r) noexcept
    {
        m_topLeft.m_x *= r.m_topLeft.m_x;
        m_topLeft.m_y *= r.m_topLeft.m_y;
        m_bottomRight.m_x *= r.m_bottomRight.m_x;
        m_bottomRight.m_y *= r.m_bottomRight.m_y;
        return *this;
    }

    inline constexpr LRectTemplate &operator/=(const LRectTemplate &r) noexcept
    {
        m_topLeft.m_x /= r.m_topLeft.m_x;
        m_topLeft.m_y /= r.m_topLeft.m_y;
        m_bottomRight.m_x /= r.m_bottomRight.m_x;
        m_bottomRight.m_y /= r.m_bottomRight.m_y;
        return *this;
    }

    inline constexpr LRectTemplate operator+(TA factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x+factor,m_topLeft.m_y+factor,m_bottomRight.m_x+factor,m_bottomRight.m_y+factor);
    }

    inline constexpr LRectTemplate operator-(TA factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x-factor,m_topLeft.m_y-factor,m_bottomRight.m_x-factor,m_bottomRight.m_y-factor);
    }

    inline constexpr LRectTemplate operator*(TA factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x*factor,m_topLeft.m_y*factor,m_bottomRight.m_x*factor,m_bottomRight.m_y*factor);
    }

    inline constexpr LRectTemplate operator/(TA factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x/factor,m_topLeft.m_y/factor,m_bottomRight.m_x/factor,m_bottomRight.m_y/factor);
    }

    inline constexpr LRectTemplate operator+(TB factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x+factor,m_topLeft.m_y+factor,m_bottomRight.m_x+factor,m_bottomRight.m_y+factor);
    }

    inline constexpr LRectTemplate operator-(TB factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x-factor,m_topLeft.m_y-factor,m_bottomRight.m_x-factor,m_bottomRight.m_y-factor);
    }

    inline constexpr LRectTemplate operator*(TB factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x*factor,m_topLeft.m_y*factor,m_bottomRight.m_x*factor,m_bottomRight.m_y*factor);
    }

    inline constexpr LRectTemplate operator/(TB factor) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x/factor,m_topLeft.m_y/factor,m_bottomRight.m_x/factor,m_bottomRight.m_y/factor);
    }

    inline constexpr LRectTemplate operator+(const LRectTemplate &r) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x+r.m_topLeft.m_x,m_topLeft.m_y+r.m_topLeft.m_y,m_bottomRight.m_x+r.m_bottomRight.m_x,m_bottomRight.m_y+r.m_bottomRight.m_y);
    }

    inline constexpr LRectTemplate operator-(const LRectTemplate &r) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x-r.m_topLeft.m_x,m_topLeft.m_y-r.m_topLeft.m_y,m_bottomRight.m_x-r.m_bottomRight.m_x,m_bottomRight.m_y-r.m_bottomRight.m_y);
    }

    inline constexpr LRectTemplate operator*(const LRectTemplate &r) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x*r.m_topLeft.m_x,m_topLeft.m_y*r.m_topLeft.m_y,m_bottomRight.m_x*r.m_bottomRight.m_x,m_bottomRight.m_y*r.m_bottomRight.m_y);
    }

    inline constexpr LRectTemplate operator/(const LRectTemplate &r) const noexcept
    {
        return LRectTemplate(m_topLeft.m_x/r.m_topLeft.m_x,m_topLeft.m_y/r.m_topLeft.m_y,m_bottomRight.m_x/r.m_bottomRight.m_x,m_bottomRight.m_y/r.m_bottomRight.m_y);
    }

    inline constexpr bool operator==(const LRectTemplate &p) const noexcept
    {
        return m_topLeft.m_x == p.m_topLeft.m_x && m_topLeft.m_y == p.m_topLeft.m_y && m_bottomRight.m_x == p.m_bottomRight.m_x && m_bottomRight.m_y == p.m_bottomRight.m_y;
    }

    inline constexpr bool operator!=(const LRectTemplate &p) const noexcept
    {
        return m_topLeft.m_x != p.m_topLeft.m_x || m_topLeft.m_y != p.m_topLeft.m_y || m_bottomRight.m_x != p.m_bottomRight.m_x || m_bottomRight.m_y != p.m_bottomRight.m_y;
    }

    /*******************************************************************/

    inline constexpr LRectTemplate &operator+=(const LPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft += p;
        m_bottomRight += p;
        return *this;
    }

    inline constexpr LRectTemplate &operator-=(const LPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft -= p;
        m_bottomRight -= p;
        return *this;
    }

    inline constexpr LRectTemplate &operator*=(const LPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft *= p;
        m_bottomRight *= p;
        return *this;
    }

    inline constexpr LRectTemplate &operator/=(const LPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft /= p;
        m_bottomRight /= p;
        return *this;
    }

private:
    friend class LRectTemplate<TB,TA>;
    LPointTemplate<TA,TB> m_topLeft,m_bottomRight;
};

#endif // LRECT_H
