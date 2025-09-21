#ifndef LRECT_H
#define LRECT_H

#include <LNamespaces.h>
#include <LPoint.h>

/**
 * @brief Template for 4D vectors
 *
 * The LRectTemplate class is a template for creating four-dimensional
 * vectors.\n The library uses this template to generate the classes LRect which
 * works with 32-bit integers and LRectF which works with 32-bit floats.
 */
template <class T>
class Louvre::LRectTemplate {
 public:
  /// Initializes the vector with (0,0,0,0).
  constexpr LRectTemplate() noexcept {}

  /// Initializes the vector with (x,y,width,height).
  constexpr LRectTemplate(T x, T y, T width, T height) noexcept
      : m_topLeft(x, y), m_bottomRight(width, height) {}

  /// Initializes the vector with
  /// (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
  template <class NA, class NB>
  constexpr LRectTemplate(const LPointTemplate<NA> &topLeft,
                          const LPointTemplate<NB> &bottomRight) noexcept
      : m_topLeft(topLeft), m_bottomRight(bottomRight) {}

  /// Initializes the vector with
  /// (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
  template <class N>
  constexpr LRectTemplate(T topLeft,
                          const LPointTemplate<N> &bottomRight) noexcept
      : m_topLeft(topLeft), m_bottomRight(bottomRight) {}

  /// Initializes the vector with
  /// (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
  template <class N>
  constexpr LRectTemplate(const LPointTemplate<N> &topLeft,
                          T bottomRight) noexcept
      : m_topLeft(topLeft), m_bottomRight(bottomRight) {}

  /// Initializes the vector with (all,all,all,all).
  constexpr LRectTemplate(T all) noexcept
      : m_topLeft(all), m_bottomRight(all) {}

  /// Initializes the vector with (p.x(),p.y(),p.x(),p.y()).
  template <class N>
  constexpr LRectTemplate(const LPointTemplate<N> &p) noexcept
      : m_topLeft(p), m_bottomRight(p) {}

  /// Copy constructor
  template <class N>
  constexpr LRectTemplate(const LRectTemplate<N> &r) noexcept
      : m_topLeft(r.pos()), m_bottomRight(r.size()) {}

  /// First vector component
  constexpr T x() const noexcept { return m_topLeft.m_x; }

  /// Second vector component
  constexpr T y() const noexcept { return m_topLeft.m_y; }

  /// Third vector component
  constexpr T w() const noexcept { return m_bottomRight.m_x; }

  /// Fourth vector component
  constexpr T h() const noexcept { return m_bottomRight.m_y; }

  /// Third vector component
  constexpr T width() const noexcept { return m_bottomRight.m_x; }

  /// Fourth vector component
  constexpr T height() const noexcept { return m_bottomRight.m_y; }

  /// The multiplication of the third and fourth component (width*height)
  constexpr T area() const noexcept { return m_bottomRight.area(); }

  /**
   * Returns true if the rectangle contains the point.
   *
   * @param point The 2D vector to check
   * @param inclusive If true, the edges of the rectangle are considered as part
   * of the rectangle.
   */
  template <class N>
  constexpr bool containsPoint(const LPointTemplate<N> &point,
                               bool inclusive = true) const noexcept {
    if (inclusive) {
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
  template <class N>
  constexpr bool intersects(const LRectTemplate<N> &rect,
                            bool inclusive = true) const noexcept {
    if (inclusive) {
      return x() <= rect.x() + rect.w() && x() + w() >= rect.x() &&
             y() <= rect.y() + rect.h() && y() + h() >= rect.y();
    }

    return x() < rect.x() + rect.w() && x() + w() > rect.x() &&
           y() < rect.y() + rect.h() && y() + h() > rect.y();
  }

  /// 2D vector given by the (x,y) components of the rectangle
  constexpr const LPointTemplate<T> &topLeft() const noexcept {
    return m_topLeft;
  }

  /// 2D vector given by the (w,h) components of the rectangle
  constexpr const LPointTemplate<T> &bottomRight() const noexcept {
    return m_bottomRight;
  }

  /// 2D vector given by the (x,y) components of the rectangle
  constexpr const LPointTemplate<T> &TL() const noexcept { return m_topLeft; }

  /// 2D vector given by the (w,h) components of the rectangle
  constexpr const LPointTemplate<T> &BR() const noexcept {
    return m_bottomRight;
  }

  /// 2D vector given by the (x,y) components of the rectangle
  constexpr const LPointTemplate<T> &pos() const noexcept { return m_topLeft; }

  /// 2D vector given by the (w,h) components of the rectangle
  constexpr const LPointTemplate<T> &size() const noexcept {
    return m_bottomRight;
  }

  /// Asigns the first component
  constexpr void setX(T x) noexcept { m_topLeft.m_x = x; }

  /// Asigns the second component
  constexpr void setY(T y) noexcept { m_topLeft.m_y = y; }

  /// Asigns the third component
  constexpr void setW(T width) noexcept { m_bottomRight.m_x = width; }

  /// Asigns the fourth component
  constexpr void setH(T height) noexcept { m_bottomRight.m_y = height; }

  /// Asigns the third component
  constexpr void setWidth(T width) noexcept { m_bottomRight.m_x = width; }

  /// Asigns the fourth component
  constexpr void setHeight(T height) noexcept { m_bottomRight.m_y = height; }

  /// Asigns the (x,y) components
  constexpr void setTL(const LPointTemplate<T> &p) noexcept { m_topLeft = p; }

  /// Asigns the (w,h) components
  constexpr void setBR(const LPointTemplate<T> &p) noexcept {
    m_bottomRight = p;
  }

  /// Asigns the (x,y) components
  constexpr void setTopLeft(const LPointTemplate<T> &p) noexcept {
    m_topLeft = p;
  }

  /// Asigns the (w,h) components
  constexpr void setBottomRight(const LPointTemplate<T> &p) noexcept {
    m_bottomRight = p;
  }

  /// Asigns the (x,y) components
  constexpr void setPos(const LPointTemplate<T> &p) noexcept { m_topLeft = p; }

  /// Asigns the (w,h) components
  constexpr void setSize(const LPointTemplate<T> &p) noexcept {
    m_bottomRight = p;
  }

  /// Returns true if the resulting rectangle has an area of 0.
  template <class N>
  constexpr bool clip(const LRectTemplate<N> &rect) noexcept {
    T x1 = x() + w();
    T y1 = y() + h();

    T rx1 = rect.x() + rect.w();
    T ry1 = rect.y() + rect.h();

    if (x() < rect.x()) setX(rect.x());

    if (x1 > rx1) x1 = rx1;

    if (y() < rect.y()) setY(rect.y());

    if (y1 > ry1) y1 = ry1;

    setW(x1 - x());
    setH(y1 - y());

    if (w() < 0) setW(0);

    if (h() < 0) setH(0);

    return (w() == 0 || h() == 0);
  }

  constexpr LRectTemplate<T> &operator+=(T factor) noexcept {
    m_topLeft.m_x += factor;
    m_topLeft.m_y += factor;
    m_bottomRight.m_x += factor;
    m_bottomRight.m_y += factor;
    return *this;
  }

  constexpr LRectTemplate<T> &operator-=(T factor) noexcept {
    m_topLeft.m_x -= factor;
    m_topLeft.m_y -= factor;
    m_bottomRight.m_x -= factor;
    m_bottomRight.m_y -= factor;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator*=(N factor) noexcept {
    m_topLeft.m_x *= factor;
    m_topLeft.m_y *= factor;
    m_bottomRight.m_x *= factor;
    m_bottomRight.m_y *= factor;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator/=(N factor) noexcept {
    m_topLeft.m_x /= factor;
    m_topLeft.m_y /= factor;
    m_bottomRight.m_x /= factor;
    m_bottomRight.m_y /= factor;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator+=(const LRectTemplate<N> &r) noexcept {
    m_topLeft.m_x += r.m_topLeft.m_x;
    m_topLeft.m_y += r.m_topLeft.m_y;
    m_bottomRight.m_x += r.m_bottomRight.m_x;
    m_bottomRight.m_y += r.m_bottomRight.m_y;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator-=(const LRectTemplate<N> &r) noexcept {
    m_topLeft.m_x -= r.m_topLeft.m_x;
    m_topLeft.m_y -= r.m_topLeft.m_y;
    m_bottomRight.m_x -= r.m_bottomRight.m_x;
    m_bottomRight.m_y -= r.m_bottomRight.m_y;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator*=(const LRectTemplate<N> &r) noexcept {
    m_topLeft.m_x *= r.m_topLeft.m_x;
    m_topLeft.m_y *= r.m_topLeft.m_y;
    m_bottomRight.m_x *= r.m_bottomRight.m_x;
    m_bottomRight.m_y *= r.m_bottomRight.m_y;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator/=(const LRectTemplate<N> &r) noexcept {
    m_topLeft.m_x /= r.m_topLeft.m_x;
    m_topLeft.m_y /= r.m_topLeft.m_y;
    m_bottomRight.m_x /= r.m_bottomRight.m_x;
    m_bottomRight.m_y /= r.m_bottomRight.m_y;
    return *this;
  }

  constexpr LRectTemplate<T> operator+(T factor) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x + factor, m_topLeft.m_y + factor,
                            m_bottomRight.m_x + factor,
                            m_bottomRight.m_y + factor);
  }

  constexpr LRectTemplate<T> operator-(T factor) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x - factor, m_topLeft.m_y - factor,
                            m_bottomRight.m_x - factor,
                            m_bottomRight.m_y - factor);
  }

  template <class N>
  constexpr LRectTemplate<T> operator*(N factor) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x * factor, m_topLeft.m_y * factor,
                            m_bottomRight.m_x * factor,
                            m_bottomRight.m_y * factor);
  }

  template <class N>
  constexpr LRectTemplate<T> operator/(N factor) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x / factor, m_topLeft.m_y / factor,
                            m_bottomRight.m_x / factor,
                            m_bottomRight.m_y / factor);
  }

  template <class N>
  constexpr LRectTemplate<T> operator+(
      const LRectTemplate<N> &r) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x + r.m_topLeft.m_x,
                            m_topLeft.m_y + r.m_topLeft.m_y,
                            m_bottomRight.m_x + r.m_bottomRight.m_x,
                            m_bottomRight.m_y + r.m_bottomRight.m_y);
  }

  template <class N>
  constexpr LRectTemplate<T> operator-(
      const LRectTemplate<N> &r) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x - r.m_topLeft.m_x,
                            m_topLeft.m_y - r.m_topLeft.m_y,
                            m_bottomRight.m_x - r.m_bottomRight.m_x,
                            m_bottomRight.m_y - r.m_bottomRight.m_y);
  }

  template <class N>
  constexpr LRectTemplate<T> operator*(
      const LRectTemplate<N> &r) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x * r.m_topLeft.m_x,
                            m_topLeft.m_y * r.m_topLeft.m_y,
                            m_bottomRight.m_x * r.m_bottomRight.m_x,
                            m_bottomRight.m_y * r.m_bottomRight.m_y);
  }

  template <class N>
  constexpr LRectTemplate<T> operator/(
      const LRectTemplate<N> &r) const noexcept {
    return LRectTemplate<T>(m_topLeft.m_x / r.m_topLeft.m_x,
                            m_topLeft.m_y / r.m_topLeft.m_y,
                            m_bottomRight.m_x / r.m_bottomRight.m_x,
                            m_bottomRight.m_y / r.m_bottomRight.m_y);
  }

  template <class N>
  constexpr bool operator==(const LRectTemplate<N> &p) const noexcept {
    return m_topLeft.m_x == p.m_topLeft.m_x &&
           m_topLeft.m_y == p.m_topLeft.m_y &&
           m_bottomRight.m_x == p.m_bottomRight.m_x &&
           m_bottomRight.m_y == p.m_bottomRight.m_y;
  }

  template <class N>
  constexpr bool operator!=(const LRectTemplate<N> &p) const noexcept {
    return m_topLeft.m_x != p.m_topLeft.m_x ||
           m_topLeft.m_y != p.m_topLeft.m_y ||
           m_bottomRight.m_x != p.m_bottomRight.m_x ||
           m_bottomRight.m_y != p.m_bottomRight.m_y;
  }

  /*******************************************************************/

  template <class N>
  constexpr LRectTemplate<T> &operator+=(const LPointTemplate<N> &p) noexcept {
    m_topLeft += p;
    m_bottomRight += p;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator-=(const LPointTemplate<N> &p) noexcept {
    m_topLeft -= p;
    m_bottomRight -= p;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator*=(const LPointTemplate<N> &p) noexcept {
    m_topLeft *= p;
    m_bottomRight *= p;
    return *this;
  }

  template <class N>
  constexpr LRectTemplate<T> &operator/=(const LPointTemplate<N> &p) noexcept {
    m_topLeft /= p;
    m_bottomRight /= p;
    return *this;
  }

 private:
  friend class LRectTemplate<Int8>;
  friend class LRectTemplate<Int16>;
  friend class LRectTemplate<Int32>;
  friend class LRectTemplate<Int64>;
  friend class LRectTemplate<UInt8>;
  friend class LRectTemplate<UInt16>;
  friend class LRectTemplate<UInt32>;
  friend class LRectTemplate<UInt64>;
  friend class LRectTemplate<Float32>;
  friend class LRectTemplate<Float64>;
  LPointTemplate<T> m_topLeft, m_bottomRight;
};

#endif  // LRECT_H
