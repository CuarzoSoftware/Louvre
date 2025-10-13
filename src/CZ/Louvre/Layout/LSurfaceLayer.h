#ifndef LLAYOUT_H
#define LLAYOUT_H

namespace CZ
{
    /**
     * @brief Enum representing surface layers.
     *
     * These layers are based on the [wlr_layer_shell](https://wayland.app/protocols/wlr-layer-shell-unstable-v1) protocol,
     * with an additional middle layer used for other surface roles.
     *
     * @note Subsurfaces (LSubsurfaceRole) and popups (LPopupRole) are always assigned
     *       to the same layer as their parent surface.
     *
     * @see LCompositor::layers(), LSurface::layer()
     */
    enum LSurfaceLayer
    {
        LLayerBackground = 0, ///< Background layer (used by LLayerRole)
        LLayerBottom     = 1, ///< Bottom layer (used by LLayerRole)
        LLayerMiddle     = 2, ///< Middle layer (used by LToplevelRole)
        LLayerTop        = 3, ///< Top layer (used by LLayerRole and LSessionLockRole)
        LLayerOverlay    = 4  ///< Overlay layer (used by LLayerRole, LCursorRole, and LDNDIconRole)
    };
}

#endif // LLAYOUT_H
