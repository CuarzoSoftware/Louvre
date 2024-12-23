/* Generated by wayland-scanner 1.23.0 */

#ifndef DRM_LEASE_V1_SERVER_PROTOCOL_H
#define DRM_LEASE_V1_SERVER_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct wl_client;
struct wl_resource;

/**
 * @page page_drm_lease_v1 The drm_lease_v1 protocol
 * @section page_ifaces_drm_lease_v1 Interfaces
 * - @subpage page_iface_wp_drm_lease_device_v1 - lease device
 * - @subpage page_iface_wp_drm_lease_connector_v1 - a leasable DRM connector
 * - @subpage page_iface_wp_drm_lease_request_v1 - DRM lease request
 * - @subpage page_iface_wp_drm_lease_v1 - a DRM lease
 * @section page_copyright_drm_lease_v1 Copyright
 * <pre>
 *
 * Copyright © 2018 NXP
 * Copyright © 2019 Status Research & Development GmbH.
 * Copyright © 2021 Xaver Hugl
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
struct wp_drm_lease_connector_v1;
struct wp_drm_lease_device_v1;
struct wp_drm_lease_request_v1;
struct wp_drm_lease_v1;

#ifndef WP_DRM_LEASE_DEVICE_V1_INTERFACE
#define WP_DRM_LEASE_DEVICE_V1_INTERFACE
/**
 * @page page_iface_wp_drm_lease_device_v1 wp_drm_lease_device_v1
 * @section page_iface_wp_drm_lease_device_v1_desc Description
 *
 * This protocol is used by Wayland compositors which act as Direct
 * Rendering Manager (DRM) masters to lease DRM resources to Wayland
 * clients.
 *
 * The compositor will advertise one wp_drm_lease_device_v1 global for each
 * DRM node. Some time after a client binds to the wp_drm_lease_device_v1
 * global, the compositor will send a drm_fd event followed by zero, one or
 * more connector events. After all currently available connectors have been
 * sent, the compositor will send a wp_drm_lease_device_v1.done event.
 *
 * When the list of connectors available for lease changes the compositor
 * will send wp_drm_lease_device_v1.connector events for added connectors and
 * wp_drm_lease_connector_v1.withdrawn events for removed connectors,
 * followed by a wp_drm_lease_device_v1.done event.
 *
 * The compositor will indicate when a device is gone by removing the global
 * via a wl_registry.global_remove event. Upon receiving this event, the
 * client should destroy any matching wp_drm_lease_device_v1 object.
 *
 * To destroy a wp_drm_lease_device_v1 object, the client must first issue
 * a release request. Upon receiving this request, the compositor will
 * immediately send a released event and destroy the object. The client must
 * continue to process and discard drm_fd and connector events until it
 * receives the released event. Upon receiving the released event, the
 * client can safely cleanup any client-side resources.
 *
 * Warning! The protocol described in this file is currently in the testing
 * phase. Backward compatible changes may be added together with the
 * corresponding interface version bump. Backward incompatible changes can
 * only be done by creating a new major version of the extension.
 * @section page_iface_wp_drm_lease_device_v1_api API
 * See @ref iface_wp_drm_lease_device_v1.
 */
/**
 * @defgroup iface_wp_drm_lease_device_v1 The wp_drm_lease_device_v1 interface
 *
 * This protocol is used by Wayland compositors which act as Direct
 * Rendering Manager (DRM) masters to lease DRM resources to Wayland
 * clients.
 *
 * The compositor will advertise one wp_drm_lease_device_v1 global for each
 * DRM node. Some time after a client binds to the wp_drm_lease_device_v1
 * global, the compositor will send a drm_fd event followed by zero, one or
 * more connector events. After all currently available connectors have been
 * sent, the compositor will send a wp_drm_lease_device_v1.done event.
 *
 * When the list of connectors available for lease changes the compositor
 * will send wp_drm_lease_device_v1.connector events for added connectors and
 * wp_drm_lease_connector_v1.withdrawn events for removed connectors,
 * followed by a wp_drm_lease_device_v1.done event.
 *
 * The compositor will indicate when a device is gone by removing the global
 * via a wl_registry.global_remove event. Upon receiving this event, the
 * client should destroy any matching wp_drm_lease_device_v1 object.
 *
 * To destroy a wp_drm_lease_device_v1 object, the client must first issue
 * a release request. Upon receiving this request, the compositor will
 * immediately send a released event and destroy the object. The client must
 * continue to process and discard drm_fd and connector events until it
 * receives the released event. Upon receiving the released event, the
 * client can safely cleanup any client-side resources.
 *
 * Warning! The protocol described in this file is currently in the testing
 * phase. Backward compatible changes may be added together with the
 * corresponding interface version bump. Backward incompatible changes can
 * only be done by creating a new major version of the extension.
 */
extern const struct wl_interface wp_drm_lease_device_v1_interface;
#endif
#ifndef WP_DRM_LEASE_CONNECTOR_V1_INTERFACE
#define WP_DRM_LEASE_CONNECTOR_V1_INTERFACE
/**
 * @page page_iface_wp_drm_lease_connector_v1 wp_drm_lease_connector_v1
 * @section page_iface_wp_drm_lease_connector_v1_desc Description
 *
 * Represents a DRM connector which is available for lease. These objects are
 * created via wp_drm_lease_device_v1.connector events, and should be passed
 * to lease requests via wp_drm_lease_request_v1.request_connector.
 * Immediately after the wp_drm_lease_connector_v1 object is created the
 * compositor will send a name, a description, a connector_id and a done
 * event. When the description is updated the compositor will send a
 * description event followed by a done event.
 * @section page_iface_wp_drm_lease_connector_v1_api API
 * See @ref iface_wp_drm_lease_connector_v1.
 */
/**
 * @defgroup iface_wp_drm_lease_connector_v1 The wp_drm_lease_connector_v1 interface
 *
 * Represents a DRM connector which is available for lease. These objects are
 * created via wp_drm_lease_device_v1.connector events, and should be passed
 * to lease requests via wp_drm_lease_request_v1.request_connector.
 * Immediately after the wp_drm_lease_connector_v1 object is created the
 * compositor will send a name, a description, a connector_id and a done
 * event. When the description is updated the compositor will send a
 * description event followed by a done event.
 */
extern const struct wl_interface wp_drm_lease_connector_v1_interface;
#endif
#ifndef WP_DRM_LEASE_REQUEST_V1_INTERFACE
#define WP_DRM_LEASE_REQUEST_V1_INTERFACE
/**
 * @page page_iface_wp_drm_lease_request_v1 wp_drm_lease_request_v1
 * @section page_iface_wp_drm_lease_request_v1_desc Description
 *
 * A client that wishes to lease DRM resources will attach the list of
 * connectors advertised with wp_drm_lease_device_v1.connector that they
 * wish to lease, then use wp_drm_lease_request_v1.submit to submit the
 * request.
 * @section page_iface_wp_drm_lease_request_v1_api API
 * See @ref iface_wp_drm_lease_request_v1.
 */
/**
 * @defgroup iface_wp_drm_lease_request_v1 The wp_drm_lease_request_v1 interface
 *
 * A client that wishes to lease DRM resources will attach the list of
 * connectors advertised with wp_drm_lease_device_v1.connector that they
 * wish to lease, then use wp_drm_lease_request_v1.submit to submit the
 * request.
 */
extern const struct wl_interface wp_drm_lease_request_v1_interface;
#endif
#ifndef WP_DRM_LEASE_V1_INTERFACE
#define WP_DRM_LEASE_V1_INTERFACE
/**
 * @page page_iface_wp_drm_lease_v1 wp_drm_lease_v1
 * @section page_iface_wp_drm_lease_v1_desc Description
 *
 * A DRM lease object is used to transfer the DRM file descriptor to the
 * client and manage the lifetime of the lease.
 *
 * Some time after the wp_drm_lease_v1 object is created, the compositor
 * will reply with the lease request's result. If the lease request is
 * granted, the compositor will send a lease_fd event. If the lease request
 * is denied, the compositor will send a finished event without a lease_fd
 * event.
 * @section page_iface_wp_drm_lease_v1_api API
 * See @ref iface_wp_drm_lease_v1.
 */
/**
 * @defgroup iface_wp_drm_lease_v1 The wp_drm_lease_v1 interface
 *
 * A DRM lease object is used to transfer the DRM file descriptor to the
 * client and manage the lifetime of the lease.
 *
 * Some time after the wp_drm_lease_v1 object is created, the compositor
 * will reply with the lease request's result. If the lease request is
 * granted, the compositor will send a lease_fd event. If the lease request
 * is denied, the compositor will send a finished event without a lease_fd
 * event.
 */
extern const struct wl_interface wp_drm_lease_v1_interface;
#endif

/**
 * @ingroup iface_wp_drm_lease_device_v1
 * @struct wp_drm_lease_device_v1_interface
 */
struct wp_drm_lease_device_v1_interface {
	/**
	 * create a lease request object
	 *
	 * Creates a lease request object.
	 *
	 * See the documentation for wp_drm_lease_request_v1 for details.
	 */
	void (*create_lease_request)(struct wl_client *client,
				     struct wl_resource *resource,
				     uint32_t id);
	/**
	 * release this object
	 *
	 * Indicates the client no longer wishes to use this object. In
	 * response the compositor will immediately send the released event
	 * and destroy this object. It can however not guarantee that the
	 * client won't receive connector events before the released event.
	 * The client must not send any requests after this one, doing so
	 * will raise a wl_display error. Existing connectors, lease
	 * request and leases will not be affected.
	 */
	void (*release)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WP_DRM_LEASE_DEVICE_V1_DRM_FD 0
#define WP_DRM_LEASE_DEVICE_V1_CONNECTOR 1
#define WP_DRM_LEASE_DEVICE_V1_DONE 2
#define WP_DRM_LEASE_DEVICE_V1_RELEASED 3

/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_DRM_FD_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_CONNECTOR_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_DONE_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_RELEASED_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_CREATE_LEASE_REQUEST_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_device_v1
 */
#define WP_DRM_LEASE_DEVICE_V1_RELEASE_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_device_v1
 * Sends an drm_fd event to the client owning the resource.
 * @param resource_ The client's resource
 * @param fd DRM file descriptor
 */
static inline void
wp_drm_lease_device_v1_send_drm_fd(struct wl_resource *resource_, int32_t fd)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_DEVICE_V1_DRM_FD, fd);
}

/**
 * @ingroup iface_wp_drm_lease_device_v1
 * Sends an connector event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_device_v1_send_connector(struct wl_resource *resource_, struct wl_resource *id)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_DEVICE_V1_CONNECTOR, id);
}

/**
 * @ingroup iface_wp_drm_lease_device_v1
 * Sends an done event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_device_v1_send_done(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_DEVICE_V1_DONE);
}

/**
 * @ingroup iface_wp_drm_lease_device_v1
 * Sends an released event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_device_v1_send_released(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_DEVICE_V1_RELEASED);
}

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * @struct wp_drm_lease_connector_v1_interface
 */
struct wp_drm_lease_connector_v1_interface {
	/**
	 * destroy connector
	 *
	 * The client may send this request to indicate that it will not
	 * use this connector. Clients are encouraged to send this after
	 * receiving the "withdrawn" event so that the server can release
	 * the resources associated with this connector offer. Neither
	 * existing lease requests nor leases will be affected.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WP_DRM_LEASE_CONNECTOR_V1_NAME 0
#define WP_DRM_LEASE_CONNECTOR_V1_DESCRIPTION 1
#define WP_DRM_LEASE_CONNECTOR_V1_CONNECTOR_ID 2
#define WP_DRM_LEASE_CONNECTOR_V1_DONE 3
#define WP_DRM_LEASE_CONNECTOR_V1_WITHDRAWN 4

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_NAME_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_DESCRIPTION_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_CONNECTOR_ID_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_DONE_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_WITHDRAWN_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 */
#define WP_DRM_LEASE_CONNECTOR_V1_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * Sends an name event to the client owning the resource.
 * @param resource_ The client's resource
 * @param name connector name
 */
static inline void
wp_drm_lease_connector_v1_send_name(struct wl_resource *resource_, const char *name)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_CONNECTOR_V1_NAME, name);
}

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * Sends an description event to the client owning the resource.
 * @param resource_ The client's resource
 * @param description connector description
 */
static inline void
wp_drm_lease_connector_v1_send_description(struct wl_resource *resource_, const char *description)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_CONNECTOR_V1_DESCRIPTION, description);
}

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * Sends an connector_id event to the client owning the resource.
 * @param resource_ The client's resource
 * @param connector_id DRM connector ID
 */
static inline void
wp_drm_lease_connector_v1_send_connector_id(struct wl_resource *resource_, uint32_t connector_id)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_CONNECTOR_V1_CONNECTOR_ID, connector_id);
}

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * Sends an done event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_connector_v1_send_done(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_CONNECTOR_V1_DONE);
}

/**
 * @ingroup iface_wp_drm_lease_connector_v1
 * Sends an withdrawn event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_connector_v1_send_withdrawn(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_CONNECTOR_V1_WITHDRAWN);
}

#ifndef WP_DRM_LEASE_REQUEST_V1_ERROR_ENUM
#define WP_DRM_LEASE_REQUEST_V1_ERROR_ENUM
enum wp_drm_lease_request_v1_error {
	/**
	 * requested a connector from a different lease device
	 */
	WP_DRM_LEASE_REQUEST_V1_ERROR_WRONG_DEVICE = 0,
	/**
	 * requested a connector twice
	 */
	WP_DRM_LEASE_REQUEST_V1_ERROR_DUPLICATE_CONNECTOR = 1,
	/**
	 * requested a lease without requesting a connector
	 */
	WP_DRM_LEASE_REQUEST_V1_ERROR_EMPTY_LEASE = 2,
};
/**
 * @ingroup iface_wp_drm_lease_request_v1
 * Validate a wp_drm_lease_request_v1 error value.
 *
 * @return true on success, false on error.
 * @ref wp_drm_lease_request_v1_error
 */
static inline bool
wp_drm_lease_request_v1_error_is_valid(uint32_t value, uint32_t version) {
	switch (value) {
	case WP_DRM_LEASE_REQUEST_V1_ERROR_WRONG_DEVICE:
		return version >= 1;
	case WP_DRM_LEASE_REQUEST_V1_ERROR_DUPLICATE_CONNECTOR:
		return version >= 1;
	case WP_DRM_LEASE_REQUEST_V1_ERROR_EMPTY_LEASE:
		return version >= 1;
	default:
		return false;
	}
}
#endif /* WP_DRM_LEASE_REQUEST_V1_ERROR_ENUM */

/**
 * @ingroup iface_wp_drm_lease_request_v1
 * @struct wp_drm_lease_request_v1_interface
 */
struct wp_drm_lease_request_v1_interface {
	/**
	 * request a connector for this lease
	 *
	 * Indicates that the client would like to lease the given
	 * connector. This is only used as a suggestion, the compositor may
	 * choose to include any resources in the lease it issues, or
	 * change the set of leased resources at any time. Compositors are
	 * however encouraged to include the requested connector and other
	 * resources necessary to drive the connected output in the lease.
	 *
	 * Requesting a connector that was created from a different lease
	 * device than this lease request raises the wrong_device error.
	 * Requesting a connector twice will raise the duplicate_connector
	 * error.
	 */
	void (*request_connector)(struct wl_client *client,
				  struct wl_resource *resource,
				  struct wl_resource *connector);
	/**
	 * submit the lease request
	 *
	 * Submits the lease request and creates a new wp_drm_lease_v1
	 * object. After calling submit the compositor will immediately
	 * destroy this object, issuing any more requests will cause a
	 * wl_display error. The compositor doesn't make any guarantees
	 * about the events of the lease object, clients cannot expect an
	 * immediate response. Not requesting any connectors before
	 * submitting the lease request will raise the empty_lease error.
	 */
	void (*submit)(struct wl_client *client,
		       struct wl_resource *resource,
		       uint32_t id);
};


/**
 * @ingroup iface_wp_drm_lease_request_v1
 */
#define WP_DRM_LEASE_REQUEST_V1_REQUEST_CONNECTOR_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_request_v1
 */
#define WP_DRM_LEASE_REQUEST_V1_SUBMIT_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_v1
 * @struct wp_drm_lease_v1_interface
 */
struct wp_drm_lease_v1_interface {
	/**
	 * destroys the lease object
	 *
	 * The client should send this to indicate that it no longer
	 * wishes to use this lease. The compositor should use
	 * drmModeRevokeLease on the appropriate file descriptor, if
	 * necessary.
	 *
	 * Upon destruction, the compositor should advertise the connector
	 * for leasing again by sending the connector event through the
	 * wp_drm_lease_device_v1 interface.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define WP_DRM_LEASE_V1_LEASE_FD 0
#define WP_DRM_LEASE_V1_FINISHED 1

/**
 * @ingroup iface_wp_drm_lease_v1
 */
#define WP_DRM_LEASE_V1_LEASE_FD_SINCE_VERSION 1
/**
 * @ingroup iface_wp_drm_lease_v1
 */
#define WP_DRM_LEASE_V1_FINISHED_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_v1
 */
#define WP_DRM_LEASE_V1_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_wp_drm_lease_v1
 * Sends an lease_fd event to the client owning the resource.
 * @param resource_ The client's resource
 * @param leased_fd leased DRM file descriptor
 */
static inline void
wp_drm_lease_v1_send_lease_fd(struct wl_resource *resource_, int32_t leased_fd)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_V1_LEASE_FD, leased_fd);
}

/**
 * @ingroup iface_wp_drm_lease_v1
 * Sends an finished event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
wp_drm_lease_v1_send_finished(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, WP_DRM_LEASE_V1_FINISHED);
}

#ifdef  __cplusplus
}
#endif

#endif
