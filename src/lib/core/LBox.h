#ifndef LBOX_H
#define LBOX_H

#include <LNamespaces.h>

namespace Louvre {
/**
 * @brief Structure representing a 2D box.
 *
 * The LBox struct defines a 2D box using four integer coordinates (x1, y1, x2,
 * y2). It is typically used to represent bounding boxes or rectangular regions
 * in 2D space.
 */
struct LBox {
  /// The x-coordinate of the top-left corner of the box.
  Int32 x1;

  /// The y-coordinate of the top-left corner of the box.
  Int32 y1;

  /// The x-coordinate of the bottom-right corner of the box.
  Int32 x2;

  /// The y-coordinate of the bottom-right corner of the box.
  Int32 y2;
};
}  // namespace Louvre

#endif  // LBOX_H
