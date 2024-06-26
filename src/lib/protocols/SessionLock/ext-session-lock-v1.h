/* Generated by wayland-scanner 1.20.0 */

#ifndef EXT_SESSION_LOCK_V1_SERVER_PROTOCOL_H
#define EXT_SESSION_LOCK_V1_SERVER_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct wl_client;
struct wl_resource;

/**
 * @page page_ext_session_lock_v1 The ext_session_lock_v1 protocol
 * secure session locking with arbitrary graphics
 *
 * @section page_desc_ext_session_lock_v1 Description
 *
 * This protocol allows for a privileged Wayland client to lock the session
 * and display arbitrary graphics while the session is locked.
 *
 * The compositor may choose to restrict this protocol to a special client
 * launched by the compositor itself or expose it to all privileged clients,
 * this is compositor policy.
 *
 * The client is responsible for performing authentication and informing the
 * compositor when the session should be unlocked. If the client dies while
 * the session is locked the session remains locked, possibly permanently
 * depending on compositor policy.
 *
 * The key words "must", "must not", "required", "shall", "shall not",
 * "should", "should not", "recommended",  "may", and "optional" in this
 * document are to be interpreted as described in IETF RFC 2119.
 *
 * Warning! The protocol described in this file is currently in the
 * testing phase. Backward compatible changes may be added together with
 * the corresponding interface version bump. Backward incompatible changes
 * can only be done by creating a new major version of the extension.
 *
 * @section page_ifaces_ext_session_lock_v1 Interfaces
 * - @subpage page_iface_ext_session_lock_manager_v1 - used to lock the session
 * - @subpage page_iface_ext_session_lock_v1 - manage lock state and create lock surfaces
 * - @subpage page_iface_ext_session_lock_surface_v1 - a surface displayed while the session is locked
 * @section page_copyright_ext_session_lock_v1 Copyright
 * <pre>
 *
 * Copyright 2021 Isaac Freund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * </pre>
 */
struct ext_session_lock_manager_v1;
struct ext_session_lock_surface_v1;
struct ext_session_lock_v1;
struct wl_output;
struct wl_surface;

#ifndef EXT_SESSION_LOCK_MANAGER_V1_INTERFACE
#define EXT_SESSION_LOCK_MANAGER_V1_INTERFACE
/**
 * @page page_iface_ext_session_lock_manager_v1 ext_session_lock_manager_v1
 * @section page_iface_ext_session_lock_manager_v1_desc Description
 *
 * This interface is used to request that the session be locked.
 * @section page_iface_ext_session_lock_manager_v1_api API
 * See @ref iface_ext_session_lock_manager_v1.
 */
/**
 * @defgroup iface_ext_session_lock_manager_v1 The ext_session_lock_manager_v1 interface
 *
 * This interface is used to request that the session be locked.
 */
extern const struct wl_interface ext_session_lock_manager_v1_interface;
#endif
#ifndef EXT_SESSION_LOCK_V1_INTERFACE
#define EXT_SESSION_LOCK_V1_INTERFACE
/**
 * @page page_iface_ext_session_lock_v1 ext_session_lock_v1
 * @section page_iface_ext_session_lock_v1_desc Description
 *
 * In response to the creation of this object the compositor must send
 * either the locked or finished event.
 *
 * The locked event indicates that the session is locked. This means
 * that the compositor must stop rendering and providing input to normal
 * clients. Instead the compositor must blank all outputs with an opaque
 * color such that their normal content is fully hidden.
 *
 * The only surfaces that should be rendered while the session is locked
 * are the lock surfaces created through this interface and optionally,
 * at the compositor's discretion, special privileged surfaces such as
 * input methods or portions of desktop shell UIs.
 *
 * The locked event must not be sent until a new "locked" frame (either
 * from a session lock surface or the compositor blanking the output) has
 * been presented on all outputs and no security sensitive normal/unlocked
 * content is possibly visible.
 *
 * The finished event should be sent immediately on creation of this
 * object if the compositor decides that the locked event will not be sent.
 *
 * The compositor may wait for the client to create and render session lock
 * surfaces before sending the locked event to avoid displaying intermediate
 * blank frames. However, it must impose a reasonable time limit if
 * waiting and send the locked event as soon as the hard requirements
 * described above can be met if the time limit expires. Clients should
 * immediately create lock surfaces for all outputs on creation of this
 * object to make this possible.
 *
 * This behavior of the locked event is required in order to prevent
 * possible race conditions with clients that wish to suspend the system
 * or similar after locking the session. Without these semantics, clients
 * triggering a suspend after receiving the locked event would race with
 * the first "locked" frame being presented and normal/unlocked frames
 * might be briefly visible as the system is resumed if the suspend
 * operation wins the race.
 *
 * If the client dies while the session is locked, the compositor must not
 * unlock the session in response. It is acceptable for the session to be
 * permanently locked if this happens. The compositor may choose to continue
 * to display the lock surfaces the client had mapped before it died or
 * alternatively fall back to a solid color, this is compositor policy.
 *
 * Compositors may also allow a secure way to recover the session, the
 * details of this are compositor policy. Compositors may allow a new
 * client to create a ext_session_lock_v1 object and take responsibility
 * for unlocking the session, they may even start a new lock client
 * instance automatically.
 * @section page_iface_ext_session_lock_v1_api API
 * See @ref iface_ext_session_lock_v1.
 */
/**
 * @defgroup iface_ext_session_lock_v1 The ext_session_lock_v1 interface
 *
 * In response to the creation of this object the compositor must send
 * either the locked or finished event.
 *
 * The locked event indicates that the session is locked. This means
 * that the compositor must stop rendering and providing input to normal
 * clients. Instead the compositor must blank all outputs with an opaque
 * color such that their normal content is fully hidden.
 *
 * The only surfaces that should be rendered while the session is locked
 * are the lock surfaces created through this interface and optionally,
 * at the compositor's discretion, special privileged surfaces such as
 * input methods or portions of desktop shell UIs.
 *
 * The locked event must not be sent until a new "locked" frame (either
 * from a session lock surface or the compositor blanking the output) has
 * been presented on all outputs and no security sensitive normal/unlocked
 * content is possibly visible.
 *
 * The finished event should be sent immediately on creation of this
 * object if the compositor decides that the locked event will not be sent.
 *
 * The compositor may wait for the client to create and render session lock
 * surfaces before sending the locked event to avoid displaying intermediate
 * blank frames. However, it must impose a reasonable time limit if
 * waiting and send the locked event as soon as the hard requirements
 * described above can be met if the time limit expires. Clients should
 * immediately create lock surfaces for all outputs on creation of this
 * object to make this possible.
 *
 * This behavior of the locked event is required in order to prevent
 * possible race conditions with clients that wish to suspend the system
 * or similar after locking the session. Without these semantics, clients
 * triggering a suspend after receiving the locked event would race with
 * the first "locked" frame being presented and normal/unlocked frames
 * might be briefly visible as the system is resumed if the suspend
 * operation wins the race.
 *
 * If the client dies while the session is locked, the compositor must not
 * unlock the session in response. It is acceptable for the session to be
 * permanently locked if this happens. The compositor may choose to continue
 * to display the lock surfaces the client had mapped before it died or
 * alternatively fall back to a solid color, this is compositor policy.
 *
 * Compositors may also allow a secure way to recover the session, the
 * details of this are compositor policy. Compositors may allow a new
 * client to create a ext_session_lock_v1 object and take responsibility
 * for unlocking the session, they may even start a new lock client
 * instance automatically.
 */
extern const struct wl_interface ext_session_lock_v1_interface;
#endif
#ifndef EXT_SESSION_LOCK_SURFACE_V1_INTERFACE
#define EXT_SESSION_LOCK_SURFACE_V1_INTERFACE
/**
 * @page page_iface_ext_session_lock_surface_v1 ext_session_lock_surface_v1
 * @section page_iface_ext_session_lock_surface_v1_desc Description
 *
 * The client may use lock surfaces to display a screensaver, render a
 * dialog to enter a password and unlock the session, or however else it
 * sees fit.
 *
 * On binding this interface the compositor will immediately send the
 * first configure event. After making the ack_configure request in
 * response to this event the client should attach and commit the first
 * buffer. Committing the surface before acking the first configure is a
 * protocol error. Committing the surface with a null buffer at any time
 * is a protocol error.
 *
 * The compositor is free to handle keyboard/pointer focus for lock
 * surfaces however it chooses. A reasonable way to do this would be to
 * give the first lock surface created keyboard focus and change keyboard
 * focus if the user clicks on other surfaces.
 * @section page_iface_ext_session_lock_surface_v1_api API
 * See @ref iface_ext_session_lock_surface_v1.
 */
/**
 * @defgroup iface_ext_session_lock_surface_v1 The ext_session_lock_surface_v1 interface
 *
 * The client may use lock surfaces to display a screensaver, render a
 * dialog to enter a password and unlock the session, or however else it
 * sees fit.
 *
 * On binding this interface the compositor will immediately send the
 * first configure event. After making the ack_configure request in
 * response to this event the client should attach and commit the first
 * buffer. Committing the surface before acking the first configure is a
 * protocol error. Committing the surface with a null buffer at any time
 * is a protocol error.
 *
 * The compositor is free to handle keyboard/pointer focus for lock
 * surfaces however it chooses. A reasonable way to do this would be to
 * give the first lock surface created keyboard focus and change keyboard
 * focus if the user clicks on other surfaces.
 */
extern const struct wl_interface ext_session_lock_surface_v1_interface;
#endif

/**
 * @ingroup iface_ext_session_lock_manager_v1
 * @struct ext_session_lock_manager_v1_interface
 */
struct ext_session_lock_manager_v1_interface {
	/**
	 * destroy the session lock manager object
	 *
	 * This informs the compositor that the session lock manager
	 * object will no longer be used. Existing objects created through
	 * this interface remain valid.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * attempt to lock the session
	 *
	 * This request creates a session lock and asks the compositor to
	 * lock the session. The compositor will send either the
	 * ext_session_lock_v1.locked or ext_session_lock_v1.finished event
	 * on the created object in response to this request.
	 */
	void (*lock)(struct wl_client *client,
		     struct wl_resource *resource,
		     uint32_t id);
};


/**
 * @ingroup iface_ext_session_lock_manager_v1
 */
#define EXT_SESSION_LOCK_MANAGER_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_ext_session_lock_manager_v1
 */
#define EXT_SESSION_LOCK_MANAGER_V1_LOCK_SINCE_VERSION 1

#ifndef EXT_SESSION_LOCK_V1_ERROR_ENUM
#define EXT_SESSION_LOCK_V1_ERROR_ENUM
enum ext_session_lock_v1_error {
	/**
	 * attempted to destroy session lock while locked
	 */
	EXT_SESSION_LOCK_V1_ERROR_INVALID_DESTROY = 0,
	/**
	 * unlock requested but locked event was never sent
	 */
	EXT_SESSION_LOCK_V1_ERROR_INVALID_UNLOCK = 1,
	/**
	 * given wl_surface already has a role
	 */
	EXT_SESSION_LOCK_V1_ERROR_ROLE = 2,
	/**
	 * given output already has a lock surface
	 */
	EXT_SESSION_LOCK_V1_ERROR_DUPLICATE_OUTPUT = 3,
	/**
	 * given wl_surface has a buffer attached or committed
	 */
	EXT_SESSION_LOCK_V1_ERROR_ALREADY_CONSTRUCTED = 4,
};
#endif /* EXT_SESSION_LOCK_V1_ERROR_ENUM */

/**
 * @ingroup iface_ext_session_lock_v1
 * @struct ext_session_lock_v1_interface
 */
struct ext_session_lock_v1_interface {
	/**
	 * destroy the session lock
	 *
	 * This informs the compositor that the lock object will no
	 * longer be used. Existing objects created through this interface
	 * remain valid.
	 *
	 * After this request is made, lock surfaces created through this
	 * object should be destroyed by the client as they will no longer
	 * be used by the compositor.
	 *
	 * It is a protocol error to make this request if the locked event
	 * was sent, the unlock_and_destroy request must be used instead.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * create a lock surface for a given output
	 *
	 * The client is expected to create lock surfaces for all outputs
	 * currently present and any new outputs as they are advertised.
	 * These won't be displayed by the compositor unless the lock is
	 * successful and the locked event is sent.
	 *
	 * Providing a wl_surface which already has a role or already has a
	 * buffer attached or committed is a protocol error, as is
	 * attaching/committing a buffer before the first
	 * ext_session_lock_surface_v1.configure event.
	 *
	 * Attempting to create more than one lock surface for a given
	 * output is a duplicate_output protocol error.
	 */
	void (*get_lock_surface)(struct wl_client *client,
				 struct wl_resource *resource,
				 uint32_t id,
				 struct wl_resource *surface,
				 struct wl_resource *output);
	/**
	 * unlock the session, destroying the object
	 *
	 * This request indicates that the session should be unlocked,
	 * for example because the user has entered their password and it
	 * has been verified by the client.
	 *
	 * This request also informs the compositor that the lock object
	 * will no longer be used and should be destroyed. Existing objects
	 * created through this interface remain valid.
	 *
	 * After this request is made, lock surfaces created through this
	 * object should be destroyed by the client as they will no longer
	 * be used by the compositor.
	 *
	 * It is a protocol error to make this request if the locked event
	 * has not been sent. In that case, the lock object must be
	 * destroyed using the destroy request.
	 *
	 * Note that a correct client that wishes to exit directly after
	 * unlocking the session must use the wl_display.sync request to
	 * ensure the server receives and processes the unlock_and_destroy
	 * request. Otherwise there is no guarantee that the server has
	 * unlocked the session due to the asynchronous nature of the
	 * Wayland protocol. For example, the server might terminate the
	 * client with a protocol error before it processes the
	 * unlock_and_destroy request.
	 */
	void (*unlock_and_destroy)(struct wl_client *client,
				   struct wl_resource *resource);
};

#define EXT_SESSION_LOCK_V1_LOCKED 0
#define EXT_SESSION_LOCK_V1_FINISHED 1

/**
 * @ingroup iface_ext_session_lock_v1
 */
#define EXT_SESSION_LOCK_V1_LOCKED_SINCE_VERSION 1
/**
 * @ingroup iface_ext_session_lock_v1
 */
#define EXT_SESSION_LOCK_V1_FINISHED_SINCE_VERSION 1

/**
 * @ingroup iface_ext_session_lock_v1
 */
#define EXT_SESSION_LOCK_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_ext_session_lock_v1
 */
#define EXT_SESSION_LOCK_V1_GET_LOCK_SURFACE_SINCE_VERSION 1
/**
 * @ingroup iface_ext_session_lock_v1
 */
#define EXT_SESSION_LOCK_V1_UNLOCK_AND_DESTROY_SINCE_VERSION 1

/**
 * @ingroup iface_ext_session_lock_v1
 * Sends an locked event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
ext_session_lock_v1_send_locked(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, EXT_SESSION_LOCK_V1_LOCKED);
}

/**
 * @ingroup iface_ext_session_lock_v1
 * Sends an finished event to the client owning the resource.
 * @param resource_ The client's resource
 */
static inline void
ext_session_lock_v1_send_finished(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, EXT_SESSION_LOCK_V1_FINISHED);
}

#ifndef EXT_SESSION_LOCK_SURFACE_V1_ERROR_ENUM
#define EXT_SESSION_LOCK_SURFACE_V1_ERROR_ENUM
enum ext_session_lock_surface_v1_error {
	/**
	 * surface committed before first ack_configure request
	 */
	EXT_SESSION_LOCK_SURFACE_V1_ERROR_COMMIT_BEFORE_FIRST_ACK = 0,
	/**
	 * surface committed with a null buffer
	 */
	EXT_SESSION_LOCK_SURFACE_V1_ERROR_NULL_BUFFER = 1,
	/**
	 * failed to match ack'd width/height
	 */
	EXT_SESSION_LOCK_SURFACE_V1_ERROR_DIMENSIONS_MISMATCH = 2,
	/**
	 * serial provided in ack_configure is invalid
	 */
	EXT_SESSION_LOCK_SURFACE_V1_ERROR_INVALID_SERIAL = 3,
};
#endif /* EXT_SESSION_LOCK_SURFACE_V1_ERROR_ENUM */

/**
 * @ingroup iface_ext_session_lock_surface_v1
 * @struct ext_session_lock_surface_v1_interface
 */
struct ext_session_lock_surface_v1_interface {
	/**
	 * destroy the lock surface object
	 *
	 * This informs the compositor that the lock surface object will
	 * no longer be used.
	 *
	 * It is recommended for a lock client to destroy lock surfaces if
	 * their corresponding wl_output global is removed.
	 *
	 * If a lock surface on an active output is destroyed before the
	 * ext_session_lock_v1.unlock_and_destroy event is sent, the
	 * compositor must fall back to rendering a solid color.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * ack a configure event
	 *
	 * When a configure event is received, if a client commits the
	 * surface in response to the configure event, then the client must
	 * make an ack_configure request sometime before the commit
	 * request, passing along the serial of the configure event.
	 *
	 * If the client receives multiple configure events before it can
	 * respond to one, it only has to ack the last configure event.
	 *
	 * A client is not required to commit immediately after sending an
	 * ack_configure request - it may even ack_configure several times
	 * before its next surface commit.
	 *
	 * A client may send multiple ack_configure requests before
	 * committing, but only the last request sent before a commit
	 * indicates which configure event the client really is responding
	 * to.
	 *
	 * Sending an ack_configure request consumes the configure event
	 * referenced by the given serial, as well as all older configure
	 * events sent on this object.
	 *
	 * It is a protocol error to issue multiple ack_configure requests
	 * referencing the same configure event or to issue an
	 * ack_configure request referencing a configure event older than
	 * the last configure event acked for a given lock surface.
	 * @param serial serial from the configure event
	 */
	void (*ack_configure)(struct wl_client *client,
			      struct wl_resource *resource,
			      uint32_t serial);
};

#define EXT_SESSION_LOCK_SURFACE_V1_CONFIGURE 0

/**
 * @ingroup iface_ext_session_lock_surface_v1
 */
#define EXT_SESSION_LOCK_SURFACE_V1_CONFIGURE_SINCE_VERSION 1

/**
 * @ingroup iface_ext_session_lock_surface_v1
 */
#define EXT_SESSION_LOCK_SURFACE_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_ext_session_lock_surface_v1
 */
#define EXT_SESSION_LOCK_SURFACE_V1_ACK_CONFIGURE_SINCE_VERSION 1

/**
 * @ingroup iface_ext_session_lock_surface_v1
 * Sends an configure event to the client owning the resource.
 * @param resource_ The client's resource
 * @param serial serial for use in ack_configure
 */
static inline void
ext_session_lock_surface_v1_send_configure(struct wl_resource *resource_, uint32_t serial, uint32_t width, uint32_t height)
{
	wl_resource_post_event(resource_, EXT_SESSION_LOCK_SURFACE_V1_CONFIGURE, serial, width, height);
}

#ifdef  __cplusplus
}
#endif

#endif
