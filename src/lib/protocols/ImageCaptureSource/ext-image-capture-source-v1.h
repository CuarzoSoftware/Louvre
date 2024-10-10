/* Generated by wayland-scanner 1.22.0 */

#ifndef EXT_IMAGE_CAPTURE_SOURCE_V1_SERVER_PROTOCOL_H
#define EXT_IMAGE_CAPTURE_SOURCE_V1_SERVER_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct wl_client;
struct wl_resource;

/**
 * @page page_ext_image_capture_source_v1 The ext_image_capture_source_v1 protocol
 * opaque image capture source objects
 *
 * @section page_desc_ext_image_capture_source_v1 Description
 *
 * This protocol serves as an intermediary between capturing protocols and
 * potential image capture sources such as outputs and toplevels.
 *
 * This protocol may be extended to support more image capture sources in the
 * future, thereby adding those image capture sources to other protocols that
 * use the image capture source object without having to modify those
 * protocols.
 *
 * Warning! The protocol described in this file is currently in the testing
 * phase. Backward compatible changes may be added together with the
 * corresponding interface version bump. Backward incompatible changes can
 * only be done by creating a new major version of the extension.
 *
 * @section page_ifaces_ext_image_capture_source_v1 Interfaces
 * - @subpage page_iface_ext_image_capture_source_v1 - opaque image capture source object
 * - @subpage page_iface_ext_output_image_capture_source_manager_v1 - image capture source manager for outputs
 * - @subpage page_iface_ext_foreign_toplevel_image_capture_source_manager_v1 - image capture source manager for foreign toplevels
 * @section page_copyright_ext_image_capture_source_v1 Copyright
 * <pre>
 *
 * Copyright © 2022 Andri Yngvason
 * Copyright © 2024 Simon Ser
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
struct ext_foreign_toplevel_handle_v1;
struct ext_foreign_toplevel_image_capture_source_manager_v1;
struct ext_image_capture_source_v1;
struct ext_output_image_capture_source_manager_v1;
struct wl_output;

#ifndef EXT_IMAGE_CAPTURE_SOURCE_V1_INTERFACE
#define EXT_IMAGE_CAPTURE_SOURCE_V1_INTERFACE
/**
 * @page page_iface_ext_image_capture_source_v1 ext_image_capture_source_v1
 * @section page_iface_ext_image_capture_source_v1_desc Description
 *
 * The image capture source object is an opaque descriptor for a capturable
 * resource.  This resource may be any sort of entity from which an image
 * may be derived.
 *
 * Note, because ext_image_capture_source_v1 objects are created from multiple
 * independent factory interfaces, the ext_image_capture_source_v1 interface is
 * frozen at version 1.
 * @section page_iface_ext_image_capture_source_v1_api API
 * See @ref iface_ext_image_capture_source_v1.
 */
/**
 * @defgroup iface_ext_image_capture_source_v1 The ext_image_capture_source_v1 interface
 *
 * The image capture source object is an opaque descriptor for a capturable
 * resource.  This resource may be any sort of entity from which an image
 * may be derived.
 *
 * Note, because ext_image_capture_source_v1 objects are created from multiple
 * independent factory interfaces, the ext_image_capture_source_v1 interface is
 * frozen at version 1.
 */
extern const struct wl_interface ext_image_capture_source_v1_interface;
#endif
#ifndef EXT_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_INTERFACE
#define EXT_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_INTERFACE
/**
 * @page page_iface_ext_output_image_capture_source_manager_v1 ext_output_image_capture_source_manager_v1
 * @section page_iface_ext_output_image_capture_source_manager_v1_desc Description
 *
 * A manager for creating image capture source objects for wl_output objects.
 * @section page_iface_ext_output_image_capture_source_manager_v1_api API
 * See @ref iface_ext_output_image_capture_source_manager_v1.
 */
/**
 * @defgroup iface_ext_output_image_capture_source_manager_v1 The ext_output_image_capture_source_manager_v1 interface
 *
 * A manager for creating image capture source objects for wl_output objects.
 */
extern const struct wl_interface ext_output_image_capture_source_manager_v1_interface;
#endif
#ifndef EXT_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER_V1_INTERFACE
#define EXT_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER_V1_INTERFACE
/**
 * @page page_iface_ext_foreign_toplevel_image_capture_source_manager_v1 ext_foreign_toplevel_image_capture_source_manager_v1
 * @section page_iface_ext_foreign_toplevel_image_capture_source_manager_v1_desc Description
 *
 * A manager for creating image capture source objects for
 * ext_foreign_toplevel_handle_v1 objects.
 * @section page_iface_ext_foreign_toplevel_image_capture_source_manager_v1_api API
 * See @ref iface_ext_foreign_toplevel_image_capture_source_manager_v1.
 */
/**
 * @defgroup iface_ext_foreign_toplevel_image_capture_source_manager_v1 The ext_foreign_toplevel_image_capture_source_manager_v1 interface
 *
 * A manager for creating image capture source objects for
 * ext_foreign_toplevel_handle_v1 objects.
 */
extern const struct wl_interface ext_foreign_toplevel_image_capture_source_manager_v1_interface;
#endif

/**
 * @ingroup iface_ext_image_capture_source_v1
 * @struct ext_image_capture_source_v1_interface
 */
struct ext_image_capture_source_v1_interface {
	/**
	 * delete this object
	 *
	 * Destroys the image capture source. This request may be sent at
	 * any time by the client.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};


/**
 * @ingroup iface_ext_image_capture_source_v1
 */
#define EXT_IMAGE_CAPTURE_SOURCE_V1_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_ext_output_image_capture_source_manager_v1
 * @struct ext_output_image_capture_source_manager_v1_interface
 */
struct ext_output_image_capture_source_manager_v1_interface {
	/**
	 * create source object for output
	 *
	 * Creates a source object for an output. Images captured from
	 * this source will show the same content as the output. Some
	 * elements may be omitted, such as cursors and overlays that have
	 * been marked as transparent to capturing.
	 */
	void (*create_source)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t source,
			      struct wl_resource *output);
	/**
	 * delete this object
	 *
	 * Destroys the manager. This request may be sent at any time by
	 * the client and objects created by the manager will remain valid
	 * after its destruction.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};


/**
 * @ingroup iface_ext_output_image_capture_source_manager_v1
 */
#define EXT_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_CREATE_SOURCE_SINCE_VERSION 1
/**
 * @ingroup iface_ext_output_image_capture_source_manager_v1
 */
#define EXT_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_ext_foreign_toplevel_image_capture_source_manager_v1
 * @struct ext_foreign_toplevel_image_capture_source_manager_v1_interface
 */
struct ext_foreign_toplevel_image_capture_source_manager_v1_interface {
	/**
	 * create source object for foreign toplevel
	 *
	 * Creates a source object for a foreign toplevel handle. Images
	 * captured from this source will show the same content as the
	 * toplevel.
	 */
	void (*create_source)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t source,
			      struct wl_resource *toplevel_handle);
	/**
	 * delete this object
	 *
	 * Destroys the manager. This request may be sent at any time by
	 * the client and objects created by the manager will remain valid
	 * after its destruction.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};


/**
 * @ingroup iface_ext_foreign_toplevel_image_capture_source_manager_v1
 */
#define EXT_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER_V1_CREATE_SOURCE_SINCE_VERSION 1
/**
 * @ingroup iface_ext_foreign_toplevel_image_capture_source_manager_v1
 */
#define EXT_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER_V1_DESTROY_SINCE_VERSION 1

#ifdef  __cplusplus
}
#endif

#endif