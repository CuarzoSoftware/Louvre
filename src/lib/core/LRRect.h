#ifndef LRRECT_H
#define LRRECT_H

#include <LRect.h>

/**
 * @brief A rectangle with rounded corners.
 *
 * This class extends LRect by adding support for rounded corners.
 */
class Louvre::LRRect : public LRect {
 public:
  /**
   * @brief Constructs a rounded rectangle.
   *
   * @param rect The base rectangle.
   * @param radTL Radius of the top-left corner.
   * @param radTR Radius of the top-right corner.
   * @param radBR Radius of the bottom-right corner.
   * @param radBL Radius of the bottom-left corner.
   */
  LRRect(const LRect& rect = {0}, Int32 radTL = 0, Int32 radTR = 0,
         Int32 radBR = 0, Int32 radBL = 0) noexcept
      : LRect(rect),
        fRadTL(radTL),
        fRadTR(radTR),
        fRadBR(radBR),
        fRadBL(radBL) {}

  /**
   * @brief Checks if the rectangle is valid.
   *
   * A rectangle is valid if its width and height are non-negative,
   * each corner radius is non-negative, and the sum of adjacent corner radii
   * does not exceed the respective width or height.
   *
   * @return true if the rectangle is valid, false otherwise.
   */
  bool isValid() const noexcept {
    return w() >= 0 && h() >= 0 && fRadTL >= 0 && fRadTR >= 0 && fRadBR >= 0 &&
           fRadBL >= 0 && fRadTL + fRadTR <= w() && fRadBL + fRadBR <= w() &&
           fRadTL + fRadBL <= h() && fRadTR + fRadBR <= h();
  }

  /**
   * @brief Compares two rounded rectangles for equality.
   *
   * @param other The rectangle to compare with.
   * @return `true` if all properties match, `false` otherwise.
   */
  constexpr bool operator==(const LRRect& other) const {
    return LRect::operator==(other) && fRadTL == other.fRadTL &&
           fRadTR == other.fRadTR && fRadBR == other.fRadBR &&
           fRadBL == other.fRadBL;
  }

  /**
   * @brief Compares two rounded rectangles for inequality.
   *
   * @param other The rectangle to compare with.
   * @return `true` if any property differs, `false` otherwise.
   */
  constexpr bool operator!=(const LRRect& other) const {
    return !(operator==(other));
  }

  /// Radius of the top-left corner.
  Int32 fRadTL;

  /// Radius of the top-right corner.
  Int32 fRadTR;

  /// Radius of the bottom-right corner.
  Int32 fRadBR;

  /// Radius of the bottom-left corner.
  Int32 fRadBL;
};

#endif  // LRRECT_H
