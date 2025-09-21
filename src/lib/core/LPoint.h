#ifndef LPOINT_H
#define LPOINT_H

#include <LNamespaces.h>

#include <type_traits>

/**
 * @brief Template for 2D vectors
 *
 * The LPointTemplate class is a template for creating two-dimensional
 * vectors.\n The library uses this template to generate the LPoint and LSize
 * classes, which work with 32-bit integers, and LPointF and LSizeF, which work
 * with 32-bit floating-point numbers.
 */
template <class T>
class Louvre::LPointTemplate {
 public:
  /// Initializes the vector with (0,0).
  constexpr LPointTemplate() noexcept : m_x(0), m_y(0) {}

  /// Initializes the vector with (x,y).
  constexpr LPointTemplate(T x, T y) noexcept : m_x(x), m_y(y) {}

  /// Initializes the vector with (n,n).
  constexpr LPointTemplate(T n) noexcept : m_x(n), m_y(n) {}

  /// Copy constructor
  template <class N>
  constexpr LPointTemplate(const LPointTemplate<N> &point) noexcept {
    m_x = point.m_x;
    m_y = point.m_y;
  }

  /// First component of the vector
  constexpr T x() const noexcept { return m_x; }

  /// Second component of the vector
  constexpr T y() const noexcept { return m_y; }

  /// First component of the vector (equivalent to x())
  constexpr T w() const noexcept { return m_x; }

  /// Second component of the vector (equivalent to y())
  constexpr T h() const noexcept { return m_y; }

  /// First component of the vector (equivalent to x())
  constexpr T width() const noexcept { return m_x; }

  /// Second component of the vector (equivalent to y())
  constexpr T height() const noexcept { return m_y; }

  /// Product of the components (x*y)
  constexpr T area() const noexcept { return m_x * m_y; }

  /// Assigns the value to the first component of the vector
  constexpr void setX(T x) noexcept { m_x = x; }

  /// Assigns the value to the second component of the vector
  constexpr void setY(T y) noexcept { m_y = y; }

  /// Assigns the value to the first component of the vector (equivalent to
  /// setX())
  constexpr void setW(T x) noexcept { m_x = x; }

  /// Assigns the value to the second component of the vector (equivalent to
  /// setY())
  constexpr void setH(T y) noexcept { m_y = y; }

  /// Assigns the value to the first component of the vector (equivalent to
  /// setX())
  constexpr void setWidth(T x) noexcept { m_x = x; }

  /// Assigns the value to the second component of the vector (equivalent to
  /// setY())
  constexpr void setHeight(T y) noexcept { m_y = y; }

  /// Assigns the second component while maintaining proportion with the first
  template <class N>
  constexpr LPointTemplate<T> constrainedToHeight(N size) const noexcept {
    LPointTemplate<T> point;

    if (size == 0 || h() == 0) return point;

    point.setW((w() * size) / h());
    point.setH(size);

    return point;
  }

  /// Distance from another point
  template <class N>
  constexpr T distanceFrom(const LPointTemplate<N> &point) const noexcept {
    return sqrt(pow(x() - point.x(), 2.f) + pow(y() - point.y(), 2.f));
  }

  constexpr LPointTemplate<T> &operator+=(T factor) noexcept {
    m_x += factor;
    m_y += factor;
    return *this;
  }

  constexpr LPointTemplate<T> &operator-=(T factor) noexcept {
    m_x -= factor;
    m_y -= factor;
    return *this;
  }

  template <class N>
  constexpr LPointTemplate<T> &operator*=(N factor) noexcept {
    m_x *= factor;
    m_y *= factor;
    return *this;
  }

  template <class N>
  constexpr LPointTemplate<T> &operator/=(N factor) noexcept {
    m_x /= factor;
    m_y /= factor;
    return *this;
  }

  constexpr LPointTemplate<T> &operator+=(const LPointTemplate &p) noexcept {
    m_x += p.m_x;
    m_y += p.m_y;
    return *this;
  }

  constexpr LPointTemplate<T> &operator-=(const LPointTemplate &p) noexcept {
    m_x -= p.m_x;
    m_y -= p.m_y;
    return *this;
  }

  template <class N>
  constexpr LPointTemplate<T> &operator*=(const LPointTemplate<N> &p) noexcept {
    m_x *= p.m_x;
    m_y *= p.m_y;
    return *this;
  }

  template <class N>
  constexpr LPointTemplate<T> &operator/=(const LPointTemplate<N> &p) noexcept {
    m_x /= p.m_x;
    m_y /= p.m_y;
    return *this;
  }

  /***************************************************************/

  constexpr LPointTemplate<T> operator+(T factor) const noexcept {
    return LPointTemplate<T>(m_x + factor, m_y + factor);
  }

  constexpr LPointTemplate<T> operator-(T factor) const noexcept {
    return LPointTemplate<T>(m_x - factor, m_y - factor);
  }

  template <class N>
  constexpr LPointTemplate<T> operator*(N factor) const noexcept {
    return LPointTemplate<T>(m_x * factor, m_y * factor);
  }

  template <class N>
  constexpr LPointTemplate<T> operator/(N factor) const noexcept {
    return LPointTemplate<T>(m_x / factor, m_y / factor);
  }

  constexpr LPointTemplate<T> operator+(
      const LPointTemplate &p) const noexcept {
    return LPointTemplate<T>(m_x + p.m_x, m_y + p.m_y);
  }

  constexpr LPointTemplate<T> operator-(
      const LPointTemplate &p) const noexcept {
    return LPointTemplate<T>(m_x - p.m_x, m_y - p.m_y);
  }

  template <class N>
  constexpr LPointTemplate<T> operator*(
      const LPointTemplate<N> &p) const noexcept {
    return LPointTemplate<T>(m_x * p.m_x, m_y * p.m_y);
  }

  template <class N>
  constexpr LPointTemplate<T> operator/(
      const LPointTemplate<N> &p) const noexcept {
    return LPointTemplate<T>(m_x / p.m_x, m_y / p.m_y);
  }

  /***************************************************************/

  template <class N>
  constexpr bool operator==(const LPointTemplate<N> &p) const noexcept {
    return m_x == p.m_x && m_y == p.m_y;
  }

  template <class N>
  constexpr bool operator!=(const LPointTemplate<N> &p) const noexcept {
    return m_x != p.m_x || m_y != p.m_y;
  }

 private:
  friend class LPointTemplate<Int8>;
  friend class LPointTemplate<Int16>;
  friend class LPointTemplate<Int32>;
  friend class LPointTemplate<Int64>;
  friend class LPointTemplate<UInt8>;
  friend class LPointTemplate<UInt16>;
  friend class LPointTemplate<UInt32>;
  friend class LPointTemplate<UInt64>;
  friend class LPointTemplate<Float32>;
  friend class LPointTemplate<Float64>;

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
  T m_x = 0;
  T m_y = 0;
};

namespace Louvre {
template <class T, class B,
          typename = std::enable_if_t<std::is_arithmetic<B>::value>>
constexpr LPointTemplate<T> operator+(B i, LPointTemplate<T> p) noexcept {
  p.setX(p.x() + i);
  p.setY(p.y() + i);
  return p;
}

template <class T, class B,
          typename = std::enable_if_t<std::is_arithmetic<B>::value>>
constexpr LPointTemplate<T> operator-(Int32 i, LPointTemplate<T> p) noexcept {
  p.setX(i - p.x());
  p.setY(i - p.y());
  return p;
}

template <class T, class B,
          typename = std::enable_if_t<std::is_arithmetic<B>::value>>
constexpr LPointTemplate<T> operator*(B i, LPointTemplate<T> p) noexcept {
  p.setX(p.x() * i);
  p.setY(p.y() * i);
  return p;
}
}  // namespace Louvre

#endif  // LPOINT_H
