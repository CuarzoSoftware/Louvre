#ifndef LLAYOUT_H
#define LLAYOUT_H

namespace Louvre {
/**
 * @brief Surface layers.
 *
 * This enum represents possible layers for a surface, indicating its position
 * in the Z-axis stacking order.
 */
enum LSurfaceLayer {
  LLayerBackground = 0,  ///< Background layer.
  LLayerBottom = 1,      ///< Bottom layer.
  LLayerMiddle = 2,      ///< Middle layer.
  LLayerTop = 3,         ///< Top layer.
  LLayerOverlay = 4      ///< Overlay layer.
};
}  // namespace Louvre

#endif  // LLAYOUT_H
