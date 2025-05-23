/* Generated by wayland-scanner 1.23.0 */

#ifndef LVR_BACKGROUND_BLUR_SERVER_PROTOCOL_H
#define LVR_BACKGROUND_BLUR_SERVER_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct wl_client;
struct wl_resource;

/**
 * @page page_lvr_background_blur The lvr_background_blur protocol
 * background blur effect for surfaces
 *
 * @section page_desc_lvr_background_blur Description
 *
 * Warning: This protocol is experimental, and backward-incompatible changes may be 
 * made in the future.
 *
 * @section page_ifaces_lvr_background_blur Interfaces
 * - @subpage page_iface_lvr_background_blur_manager - background blur manager
 * - @subpage page_iface_lvr_background_blur - blur effect applied to the background of a surface
 * @section page_copyright_lvr_background_blur Copyright
 * <pre>
 *
 * Copyright © 2025 Eduardo Hopperdietzel
 * Copyright © 2025 Fox2Code
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct lvr_background_blur;
struct lvr_background_blur_manager;
struct lvr_svg_path;
struct wl_region;
struct wl_surface;

#ifndef LVR_BACKGROUND_BLUR_MANAGER_INTERFACE
#define LVR_BACKGROUND_BLUR_MANAGER_INTERFACE
/**
 * @page page_iface_lvr_background_blur_manager lvr_background_blur_manager
 * @section page_iface_lvr_background_blur_manager_desc Description
 *
 * This interface allows a compositor to declare support for blurring surface 
 * backgrounds.
 *
 * Immediately after the client binds to this global, the compositor must send a 
 * masking_capabilities event. Once this event is sent, the client is permitted
 * to issue get_background_blur requests.
 * @section page_iface_lvr_background_blur_manager_api API
 * See @ref iface_lvr_background_blur_manager.
 */
/**
 * @defgroup iface_lvr_background_blur_manager The lvr_background_blur_manager interface
 *
 * This interface allows a compositor to declare support for blurring surface 
 * backgrounds.
 *
 * Immediately after the client binds to this global, the compositor must send a 
 * masking_capabilities event. Once this event is sent, the client is permitted
 * to issue get_background_blur requests.
 */
extern const struct wl_interface lvr_background_blur_manager_interface;
#endif
#ifndef LVR_BACKGROUND_BLUR_INTERFACE
#define LVR_BACKGROUND_BLUR_INTERFACE
/**
 * @page page_iface_lvr_background_blur lvr_background_blur
 * @section page_iface_lvr_background_blur_desc Description
 *
 * The set_region request defines the blur area in local surface coordinates.
 *
 * An optional mask can be specified using either set_round_rect_mask or 
 * set_svg_path_mask, if supported by the compositor. Setting one mask replaces 
 * the other.
 *
 * The final blur area is determined by the intersection of the surface bounds, 
 * the blur region, and the optional mask.
 *
 * Providing only a mask without a region is equivalent to defining an empty region.
 *
 * The client should avoid rendering fully opaque content within the blur area while 
 * it is enabled. 
 * Ideally, if the region is 100% transparent, the client should mark it as invisible 
 * using the lvr_invisible_region protocol.
 *
 * For the blur effect to take effect, the compositor must send an enabled state event, 
 * a configure event, and the client must acknowledge and commit either a null or 
 * non-empty region.
 *
 * Thereafter, the compositor should maintain the effect state and color hint until 
 * the client acknowledges and commits a new configuration.
 *
 * Initially, the blur area is an empty region (no blur).
 * @section page_iface_lvr_background_blur_api API
 * See @ref iface_lvr_background_blur.
 */
/**
 * @defgroup iface_lvr_background_blur The lvr_background_blur interface
 *
 * The set_region request defines the blur area in local surface coordinates.
 *
 * An optional mask can be specified using either set_round_rect_mask or 
 * set_svg_path_mask, if supported by the compositor. Setting one mask replaces 
 * the other.
 *
 * The final blur area is determined by the intersection of the surface bounds, 
 * the blur region, and the optional mask.
 *
 * Providing only a mask without a region is equivalent to defining an empty region.
 *
 * The client should avoid rendering fully opaque content within the blur area while 
 * it is enabled. 
 * Ideally, if the region is 100% transparent, the client should mark it as invisible 
 * using the lvr_invisible_region protocol.
 *
 * For the blur effect to take effect, the compositor must send an enabled state event, 
 * a configure event, and the client must acknowledge and commit either a null or 
 * non-empty region.
 *
 * Thereafter, the compositor should maintain the effect state and color hint until 
 * the client acknowledges and commits a new configuration.
 *
 * Initially, the blur area is an empty region (no blur).
 */
extern const struct wl_interface lvr_background_blur_interface;
#endif

#ifndef LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_ENUM
#define LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_ENUM
/**
 * @ingroup iface_lvr_background_blur_manager
 * masking capabilities
 *
 * Defines the masking options supported by the compositor.
 */
enum lvr_background_blur_manager_masking_capabilities {
	/**
	 * no masking options available
	 *
	 * The client can only define the blur region using
	 * lvr_background_blur.set_region.
	 */
	LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_NONE = 0,
	/**
	 * support for rounded rectangle masks
	 *
	 * The client can further clip the blur region using
	 * lvr_background_blur.set_round_rect_mask.
	 */
	LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_ROUND_RECT = 1,
	/**
	 * support for SVG path masks
	 *
	 * The client can further clip the blur region using
	 * lvr_background_blur.set_svg_path_mask.
	 */
	LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_SVG_PATH = 2,
};
/**
 * @ingroup iface_lvr_background_blur_manager
 * Validate a lvr_background_blur_manager masking_capabilities value.
 *
 * @return true on success, false on error.
 * @ref lvr_background_blur_manager_masking_capabilities
 */
static inline bool
lvr_background_blur_manager_masking_capabilities_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_NONE:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_ROUND_RECT:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_SVG_PATH:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_ENUM */

#ifndef LVR_BACKGROUND_BLUR_MANAGER_ERROR_ENUM
#define LVR_BACKGROUND_BLUR_MANAGER_ERROR_ENUM
enum lvr_background_blur_manager_error {
	/**
	 * the surface already has an associated background blur object
	 */
	LVR_BACKGROUND_BLUR_MANAGER_ERROR_ALREADY_CONSTRUCTED = 0,
};
/**
 * @ingroup iface_lvr_background_blur_manager
 * Validate a lvr_background_blur_manager error value.
 *
 * @return true on success, false on error.
 * @ref lvr_background_blur_manager_error
 */
static inline bool
lvr_background_blur_manager_error_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case LVR_BACKGROUND_BLUR_MANAGER_ERROR_ALREADY_CONSTRUCTED:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* LVR_BACKGROUND_BLUR_MANAGER_ERROR_ENUM */

/**
 * @ingroup iface_lvr_background_blur_manager
 * @struct lvr_background_blur_manager_interface
 */
struct lvr_background_blur_manager_interface {
	/**
	 * create a new background blur object for a given surface
	 *
	 * The surface must not already have an associated background
	 * blur object, otherwise the already_constructed error is emitted.
	 *
	 * Immediately after the object is created, the compositor will
	 * send a lvr_background_blur.set_state event, a
	 * lvr_background_blur.set_color_hint event, and finally a
	 * lvr_background_blur.configure event.
	 *
	 * Surfaces must acknowledge the configure event and may then
	 * define the blur region and optionally a supported clipping mask
	 * before the next commit.
	 *
	 * The client may ignore all configure events except for the last
	 * one.
	 *
	 * A client can send multiple lvr_background_blur.ack_configure
	 * requests before committing, but only the last request sent prior
	 * to the commit indicates which configure event the client is
	 * responding to.
	 */
	void (*get_background_blur)(struct wl_client *client,
				    struct wl_resource *resource,
				    uint32_t id,
				    struct wl_resource *surface);
	/**
	 * destroy the background blur manager object
	 *
	 * This doesn't destroy objects created with the manager.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES 0

/**
 * @ingroup iface_lvr_background_blur_manager
 */
#define LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES_SINCE_VERSION 1

/**
 * @ingroup iface_lvr_background_blur_manager
 */
#define LVR_BACKGROUND_BLUR_MANAGER_GET_BACKGROUND_BLUR_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur_manager
 */
#define LVR_BACKGROUND_BLUR_MANAGER_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_lvr_background_blur_manager
 * Sends an masking_capabilities event to the client owning the resource.
 * @param resource_ The client's resource
 * @param capabilities bitmask representing the supported masking capabilities
 */
static inline void
lvr_background_blur_manager_send_masking_capabilities(struct wl_resource *resource_, uint32_t capabilities)
{
	wl_resource_post_event(resource_, LVR_BACKGROUND_BLUR_MANAGER_MASKING_CAPABILITIES, capabilities);
}

#ifndef LVR_BACKGROUND_BLUR_ERROR_ENUM
#define LVR_BACKGROUND_BLUR_ERROR_ENUM
enum lvr_background_blur_error {
	/**
	 * surface destroyed before object
	 */
	LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE = 0,
	/**
	 * invalid ack serial
	 */
	LVR_BACKGROUND_BLUR_ERROR_INVALID_SERIAL = 1,
	/**
	 * the mask is not supported by the compositor
	 */
	LVR_BACKGROUND_BLUR_ERROR_UNSUPPORTED_MASK = 2,
	/**
	 * invalid round rect
	 */
	LVR_BACKGROUND_BLUR_ERROR_INVALID_ROUND_RECT = 3,
	/**
	 * invalid svg path
	 */
	LVR_BACKGROUND_BLUR_ERROR_INVALID_SVG_PATH = 4,
};
/**
 * @ingroup iface_lvr_background_blur
 * Validate a lvr_background_blur error value.
 *
 * @return true on success, false on error.
 * @ref lvr_background_blur_error
 */
static inline bool
lvr_background_blur_error_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_ERROR_INVALID_SERIAL:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_ERROR_UNSUPPORTED_MASK:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_ERROR_INVALID_ROUND_RECT:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_ERROR_INVALID_SVG_PATH:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* LVR_BACKGROUND_BLUR_ERROR_ENUM */

#ifndef LVR_BACKGROUND_BLUR_STATE_ENUM
#define LVR_BACKGROUND_BLUR_STATE_ENUM
enum lvr_background_blur_state {
	/**
	 * the blur effect is not displayed by the compositor
	 */
	LVR_BACKGROUND_BLUR_STATE_DISABLED = 0,
	/**
	 * the blur effect is displayed by the compositor
	 */
	LVR_BACKGROUND_BLUR_STATE_ENABLED = 1,
};
/**
 * @ingroup iface_lvr_background_blur
 * Validate a lvr_background_blur state value.
 *
 * @return true on success, false on error.
 * @ref lvr_background_blur_state
 */
static inline bool
lvr_background_blur_state_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case LVR_BACKGROUND_BLUR_STATE_DISABLED:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_STATE_ENABLED:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* LVR_BACKGROUND_BLUR_STATE_ENUM */

#ifndef LVR_BACKGROUND_BLUR_COLOR_HINT_ENUM
#define LVR_BACKGROUND_BLUR_COLOR_HINT_ENUM
enum lvr_background_blur_color_hint {
	/**
	 * the blur effect color is unknown
	 */
	LVR_BACKGROUND_BLUR_COLOR_HINT_UNKNOWN = 0,
	/**
	 * the blur effect has a dark tone
	 */
	LVR_BACKGROUND_BLUR_COLOR_HINT_DARK = 1,
	/**
	 * the blur effect has a light tone
	 */
	LVR_BACKGROUND_BLUR_COLOR_HINT_LIGHT = 2,
};
/**
 * @ingroup iface_lvr_background_blur
 * Validate a lvr_background_blur color_hint value.
 *
 * @return true on success, false on error.
 * @ref lvr_background_blur_color_hint
 */
static inline bool
lvr_background_blur_color_hint_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case LVR_BACKGROUND_BLUR_COLOR_HINT_UNKNOWN:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_COLOR_HINT_DARK:
		return version >= 1;
	case LVR_BACKGROUND_BLUR_COLOR_HINT_LIGHT:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* LVR_BACKGROUND_BLUR_COLOR_HINT_ENUM */

/**
 * @ingroup iface_lvr_background_blur
 * @struct lvr_background_blur_interface
 */
struct lvr_background_blur_interface {
	/**
	 * destroy the background blur object
	 *
	 * Switch back to a mode without background blur at the next
	 * commit.
	 *
	 * The object must be destroyed before the associated surface
	 * otherwise the destroyed_surface error is emitted.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * set the blurred region
	 *
	 * This is a double-buffered operation, refer to
	 * wl_surface.commit.
	 *
	 * Setting the pending blur region has copy semantics, allowing the
	 * wl_region object to be destroyed immediately.
	 *
	 * The region is defined in local surface coordinates and may
	 * extend beyond the surface bounds.
	 *
	 * Setting a null region means that the blur area automatically
	 * adapts to the entire surface dimensions.
	 *
	 * The region is not considered a mask and is always supported by
	 * the compositor.
	 *
	 * The initial blur area is an empty region (no blur).
	 */
	void (*set_region)(struct wl_client *client,
			   struct wl_resource *resource,
			   struct wl_resource *region);
	/**
	 * ack a pending configuration
	 *
	 * Acknowledging a serial that has not been sent, or one that has
	 * already been acknowledged, triggers the invalid_serial error.
	 * @param serial serial of the configure event
	 */
	void (*ack_configure)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t serial);
	/**
	 * clear any previously set mask
	 *
	 * This is a double-buffered operation, refer to
	 * wl_surface.commit.
	 *
	 * This doesn't affect the blur region set with set_region.
	 *
	 * If no clipping was set previously, this is a no-op.
	 */
	void (*clear_mask)(struct wl_client *client,
			   struct wl_resource *resource);
	/**
	 * mask the blur region using a rounded rectangle
	 *
	 * This is a double-buffered operation, refer to
	 * wl_surface.commit.
	 *
	 * The rounded rectangle is defined in local surface coordinates
	 * and may extend beyond the surface bounds.
	 *
	 * If the width, height, or radius is negative, or if the sum of
	 * radii for an edge exceeds the corresponding axis dimension, the
	 * invalid_round_rect error is emitted.
	 *
	 * If the compositor has not advertised support for this type of
	 * mask, the unsupported_mask error is emitted.
	 *
	 * This request overrides any previously set mask.
	 */
	void (*set_round_rect_mask)(struct wl_client *client,
				    struct wl_resource *resource,
				    int32_t x,
				    int32_t y,
				    int32_t width,
				    int32_t height,
				    int32_t radTL,
				    int32_t radTR,
				    int32_t radBR,
				    int32_t radBL);
	/**
	 * mask the blur region using an SVG path
	 *
	 * This is a double-buffered operation, refer to
	 * wl_surface.commit.
	 *
	 * Setting the SVG path mask has copy semantics, allowing the
	 * lvr_svg_path object to be destroyed immediately.
	 *
	 * The SVG path is defined in local surface coordinates and may
	 * extend beyond the surface bounds.
	 *
	 * Setting an unconstructed or invalid path triggers the
	 * invalid_svg_path error.
	 *
	 * If the compositor has not advertised support for this type of
	 * mask, the unsupported_mask error is emitted.
	 *
	 * This request replaces any previously set clipping.
	 * @param path the SVG path mask
	 */
	void (*set_svg_path_mask)(struct wl_client *client,
				  struct wl_resource *resource,
				  struct wl_resource *path);
};

#define LVR_BACKGROUND_BLUR_STATE 0
#define LVR_BACKGROUND_BLUR_COLOR_HINT 1
#define LVR_BACKGROUND_BLUR_CONFIGURE 2

/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_STATE_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_COLOR_HINT_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_CONFIGURE_SINCE_VERSION 1

/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_SET_REGION_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_ACK_CONFIGURE_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_CLEAR_MASK_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_SET_ROUND_RECT_MASK_SINCE_VERSION 1
/**
 * @ingroup iface_lvr_background_blur
 */
#define LVR_BACKGROUND_BLUR_SET_SVG_PATH_MASK_SINCE_VERSION 1

/**
 * @ingroup iface_lvr_background_blur
 * Sends an state event to the client owning the resource.
 * @param resource_ The client's resource
 * @param state the state of the blur effect.
 */
static inline void
lvr_background_blur_send_state(struct wl_resource *resource_, uint32_t state)
{
	wl_resource_post_event(resource_, LVR_BACKGROUND_BLUR_STATE, state);
}

/**
 * @ingroup iface_lvr_background_blur
 * Sends an color_hint event to the client owning the resource.
 * @param resource_ The client's resource
 * @param color_hint the color tone of the blur effect.
 */
static inline void
lvr_background_blur_send_color_hint(struct wl_resource *resource_, uint32_t color_hint)
{
	wl_resource_post_event(resource_, LVR_BACKGROUND_BLUR_COLOR_HINT, color_hint);
}

/**
 * @ingroup iface_lvr_background_blur
 * Sends an configure event to the client owning the resource.
 * @param resource_ The client's resource
 * @param serial serial of the configure event
 */
static inline void
lvr_background_blur_send_configure(struct wl_resource *resource_, uint32_t serial)
{
	wl_resource_post_event(resource_, LVR_BACKGROUND_BLUR_CONFIGURE, serial);
}

#ifdef  __cplusplus
}
#endif

#endif
