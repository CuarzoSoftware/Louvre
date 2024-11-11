#ifndef LCLIENT_H
#define LCLIENT_H

#include <LFactoryObject.h>
#include <LPointerEnterEvent.h>
#include <LPointerLeaveEvent.h>
#include <LPointerMoveEvent.h>
#include <LPointerButtonEvent.h>
#include <LPointerScrollEvent.h>
#include <LPointerSwipeBeginEvent.h>
#include <LPointerSwipeUpdateEvent.h>
#include <LPointerSwipeEndEvent.h>
#include <LPointerPinchBeginEvent.h>
#include <LPointerPinchUpdateEvent.h>
#include <LPointerPinchEndEvent.h>
#include <LPointerHoldBeginEvent.h>
#include <LPointerHoldEndEvent.h>
#include <LKeyboardEnterEvent.h>
#include <LKeyboardLeaveEvent.h>
#include <LKeyboardKeyEvent.h>
#include <LKeyboardModifiersEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchCancelEvent.h>

/**
 * @brief Representation of a Wayland client.
 *
 * The LClient class represents a Wayland client connected to the compositor.\n
 * It allows managing the client connection, accessing its resources created through various Wayland protocols,
 * handling ping/pong events, and more.
 */
class Louvre::LClient : public LFactoryObject
{
public:
    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LClient;

    /**
     * @brief Pointer event history.
     */
    struct PointerHistory
    {
        /// Sent when a surface acquires pointer focus.
        LPointerEnterEvent enter;

        /// Sent when a surface loses pointer focus.
        LPointerLeaveEvent leave;

        /// Ring buffer of the last 5 pointer button events.
        LPointerButtonEvent button[5];

        /// Current index of the pointer button ring buffer.
        UInt8 buttonIndex { 0 };

        LPointerSwipeBeginEvent swipeBegin; ///< Sent when a pointer swipe begins.
        LPointerSwipeEndEvent swipeEnd; ///< Sent when a pointer swipe ends.
        LPointerPinchBeginEvent pinchBegin; ///< Sent when a pointer pinch begins.
        LPointerPinchEndEvent pinchEnd; ///< Sent when a pointer pinch ends.
        LPointerHoldBeginEvent holdBegin; ///< Sent when a pointer hold begins.
        LPointerHoldEndEvent holdEnd; ///< Sent when a pointer hold ends.
    };

    /**
     * @brief Keyboard event history.
     */
    struct KeyboardHistory
    {
        LKeyboardEnterEvent enter; ///< Sent when a surface acquires keyboard focus.

        /// Ring buffer of the last 5 keyboard key events.
        LKeyboardKeyEvent key[5];

        /// Current index of the keyboard key ring buffer.
        UInt8 keyIndex { 0 };

        LKeyboardLeaveEvent leave; ///< Sent when a surface loses keyboard focus.
        LKeyboardModifiersEvent modifiers; ///< Sent when keyboard modifiers change.
    };

    /**
     * @brief Touch event history.
     *
     * A vector is used to store the last down and up events for specific touch points.\n
     * This is because the number of touch points is variable, but they are always re-used.
     */
    struct TouchHistory
    {
        std::vector<LTouchDownEvent> down; ///< Vector of the last touch down events.
        std::vector<LTouchUpEvent> up; ///< Vector of the last touch up events.
    };

    /**
     * @brief Structure containing the last events sent to the client.
     *
     * This structure only contains event types that have a serial number in
     * their respective Wayland protocol interface.
     */
    struct EventHistory
    {
        /// Pointer event history.
        PointerHistory pointer;

        /// Keyboard event history.
        KeyboardHistory keyboard;

        /// Touch event history.
        TouchHistory touch;
    };

    /**
     * @brief Constructor of the LClient class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LClient(const void *params) noexcept;

    LCLASS_NO_COPY(LClient)

    /**
     * @brief Destructor of the LClient class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LClient();

    /**
     * @brief Retrieves Unix credentials of the client
     *
     * This method allows you to retrieve the process ID (PID), user ID (UID), and group ID (GID) of the client.
     *
     * @see [wl_client_get_credentials](https://wayland.freedesktop.org/docs/html/apc.html#Server-structwl__client_1a82a97cb3a66c1c56826a09a7b42453d9)
     *
     * @param pid A pointer to store the process ID (PID), or `nullptr` if not needed.
     * @param uid A pointer to store the user ID (UID), or `nullptr` if not needed.
     * @param gid A pointer to store the group ID (GID), or `nullptr` if not needed.
     */
    void credentials(pid_t *pid, uid_t *uid = nullptr, gid_t *gid = nullptr) const noexcept;

    /**
     * @brief Sends a ping event to the client.
     *
     * This method sends a serial number to the client, which is expected to acknowledge it by invoking the `pong()` virtual method.\n
     * It is primarily used to detect if a client is unresponsive.
     *
     * @note Not all clients may support this mechanism. If the client does not support it, this method returns `false`, and you should not wait for a pong() response.
     *
     * @param serial The serial number that the client must ACK when calling pong().
     *
     * @return `true` if the Ping event was sent successfully, `false` if the client does not support this mechanism.
     */
    bool ping(UInt32 serial) const noexcept;

    /**
     * @brief Client response to a ping() event.
     *
     * Override this virtual method if you want to be notified when a client responds to a ping() event.
     *
     * @param serial The same serial number passed in ping().
     *
     * @par Default Implementation
     * @snippet LClientDefault.cpp pong
     */
    virtual void pong(UInt32 serial);

    /**
     * @brief Native `wl_client` struct of the client.
     *
     * This method returns a pointer to the native `wl_client` struct associated with the client. The `wl_client` struct is part of the original Wayland library.
     */
    wl_client *client() const noexcept;

    /**
     * @brief Immediately flushes pending events.
     *
     * Use this method to forcefully and immediately send any pending messages to the client.
     */
    void flush() noexcept;

    /**
     * @brief Terminates the client connection with the compositor.
     *
     * This method terminates the client's connection with the compositor, equivalent to invoking `wl_client_destroy()`.\n
     * For safety reasons, it does not occur immediately but at the end of a main loop iteration.
     *
     * @note All resources created by the client are automatically destroyed in reverse order of creation.
     */
    void destroyLater() noexcept;

    /**
     * @brief Retrieves the client's event history.
     *
     * This method returns a reference to a structure containing the most recent events that the compositor has sent to the client.\n
     * Initially, all events have a serial number of 0, indicating that the compositor has never sent that specific event type.
     *
     * @see findEventBySerial()
     *
     * @return A reference to a const EventHistory structure.
     */
    const EventHistory &eventHistory() const noexcept;

    /**
     * @brief Finds an event sent by the compositor matching the given serial number.
     *
     * This method searches for an event sent by the compositor with the specified serial number in the client's eventHistory().\n
     * If a matching event is found, a pointer to that event is returned, otherwise, `nullptr`.
     *
     * @note The returned event pointer must not be manually deleted.
     *
     * @param serial The serial number to search for.
     * @return A pointer to the event matching the serial number sent by the compositor, or `nullptr` if not found.
     */
    const LEvent *findEventBySerial(UInt32 serial) const noexcept;

    /**
     * @brief Retrieves the last cursor requested by the client.
     *
     * This method returns a reference to the last cursor that the client requested to set.
     *
     * @see LPointer::setCursorRequest()
     *
     * @return A reference to the last cursor requested by the client.
     */
    const LClientCursor &lastCursorRequest() const noexcept;

    /**
     * Resources created when the client binds to the [wl_output](https://wayland.app/protocols/wayland#wl_output) global.\n
     *
     * @note Louvre internally creates a [wl_output](https://wayland.app/protocols/wayland#wl_output) global for each initialized output.
     */
    const std::vector<Protocols::Wayland::GOutput*>&outputGlobals() const noexcept;

    /**
     * Resources created when the client binds to
     * the [wl_compositor](https://wayland.app/protocols/wayland#wl_compositor)
     * singleton global of the Wayland protocol.
     */
    const std::vector<Protocols::Wayland::GCompositor*>&compositorGlobals() const noexcept;

    /**
     * Resources created when the client binds to
     * the [wl_subcompositor](https://wayland.app/protocols/wayland#wl_subcompositor)
     * global of the Wayland protocol.
     */
    const std::vector<Protocols::Wayland::GSubcompositor*>&subcompositorGlobals() const noexcept;

    /**
     * Resources created when the client binds to
     * the [wl_seat](https://wayland.app/protocols/wayland#wl_seat)
     * global of the Wayland protocol.
     */
    const std::vector<Protocols::Wayland::GSeat*>&seatGlobals() const noexcept;

    /**
     * Resource created when the client binds to
     * the [wl_data_device_manager](https://wayland.app/protocols/wayland#wl_data_device_manager)
     * singleton global of the Wayland protocol.
     */
    const std::vector<Protocols::Wayland::GDataDeviceManager*> &dataDeviceManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [xdg_wm_base](https://wayland.app/protocols/xdg-shell#xdg_wm_base) global of the XdgShell protocol.
     */
    const std::vector<Protocols::XdgShell::GXdgWmBase *> &xdgWmBaseGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zxdg_decoration_manager_v1](https://wayland.app/protocols/xdg-decoration-unstable-v1#zxdg_decoration_manager_v1) global
     * of the XdgDecoration protocol.
     *
     * The [zxdg_decoration_manager_v1](https://wayland.app/protocols/xdg-decoration-unstable-v1#zxdg_decoration_manager_v1)
     * interface allows the client and the compositor negotiate who should draw the decoration of toplevel surfaces.
     */
    const std::vector<Protocols::XdgDecoration::GXdgDecorationManager*> &xdgDecorationManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_presentation](https://wayland.app/protocols/presentation-time#wp_presentation) global of the
     * PresentationTime protocol.
     */
    const std::vector<Protocols::PresentationTime::GPresentation*> &presentationTimeGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwp_linux_dmabuf_v1](https://wayland.app/protocols/linux-dmabuf-unstable-v1#zwp_linux_dmabuf_v1) global
     * of the LinuxDMA-BUF protocol.
     */
    const std::vector<Protocols::LinuxDMABuf::GLinuxDMABuf*> &linuxDMABufGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_viewporter](https://wayland.app/protocols/viewporter#wp_viewporter) global
     * of the Viewporter protocol.
     */
    const std::vector<Protocols::Viewporter::GViewporter*> &viewporterGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_fractional_scale_manager_v1](https://wayland.app/protocols/fractional-scale-v1#wp_fractional_scale_manager_v1) global
     * of the FractionalScale protocol.
     */
    const std::vector<Protocols::FractionalScale::GFractionalScaleManager*> &fractionalScaleManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwlr_gamma_control_manager_v1](https://wayland.app/protocols/wlr-gamma-control-unstable-v1#zwlr_gamma_control_manager_v1) global
     * of the wlroots Gamma Control protocol.
     */
    const std::vector<Protocols::GammaControl::GGammaControlManager*> &gammaControlManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_tearing_control_manager_v1](https://wayland.app/protocols/tearing-control-v1#wp_tearing_control_manager_v1) global
     * of the Tearing Control protocol.
     */
    const std::vector<Protocols::TearingControl::GTearingControlManager*> &tearingControlManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwp_relative_pointer_manager_v1](https://wayland.app/protocols/relative-pointer-unstable-v1) global
     * of the Relative Pointer protocol.
     */
    const std::vector<Protocols::RelativePointer::GRelativePointerManager*> &relativePointerManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwp_pointer_gestures_v1](https://wayland.app/protocols/pointer-gestures-unstable-v1#zwp_pointer_gestures_v1) global
     * of the Pointer Gestures protocol.
     */
    const std::vector<Protocols::PointerGestures::GPointerGestures*> &pointerGesturesGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [ext_session_lock_manager_v1](https://wayland.app/protocols/ext-session-lock-v1#ext_session_lock_manager_v1) global
     * of the Session Lock protocol.
     */
    const std::vector<Protocols::SessionLock::GSessionLockManager*> &sessionLockManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwp_pointer_constraints_v1](https://wayland.app/protocols/pointer-constraints-unstable-v1#zwp_pointer_constraints_v1) global
     * of the Pointer Constraints protocol.
     */
    const std::vector<Protocols::PointerConstraints::GPointerConstraints*> &pointerConstraintsGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zxdg_output_manager_v1](https://wayland.app/protocols/xdg-output-unstable-v1#zxdg_output_manager_v1) global
     * of the XDG Output protocol.
     */
    const std::vector<Protocols::XdgOutput::GXdgOutputManager*> &xdgOutputManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwlr_screencopy_manager_v1](https://wayland.app/protocols/wlr-screencopy-unstable-v1#zwlr_screencopy_manager_v1) global
     * of the wlroots Screen Copy protocol.
     */
    const std::vector<Protocols::ScreenCopy::GScreenCopyManager*> &screenCopyManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [ext_output_image_capture_source_manager_v1](https://wayland.app/protocols/ext-image-capture-source-v1#ext_output_image_capture_source_manager_v1) global
     * of the Image Capture Source protocol.
     */
    const std::vector<Protocols::ImageCaptureSource::GOutputImageCaptureSourceManager*> &outputImageCaptureSourceManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [ext_foreign_toplevel_image_capture_source_manager_v1](https://wayland.app/protocols/ext-image-capture-source-v1#ext_foreign_toplevel_image_capture_source_manager_v1) global
     * of the Image Capture Source protocol.
     */
    const std::vector<Protocols::ImageCaptureSource::GForeignToplevelImageCaptureSourceManager*> &foreignToplevelImageCaptureSourceManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwlr_layer_shell_v1](https://wayland.app/protocols/wlr-layer-shell-unstable-v1#zwlr_layer_shell_v1) global
     * of the wlroots Layer Shell protocol.
     */
    const std::vector<Protocols::LayerShell::GLayerShell*> &layerShellGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwlr_foreign_toplevel_manager_v1](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1#zwlr_foreign_toplevel_manager_v1) global
     * of the wlroots Foreign Toplevel Management protocol.
     */
    const std::vector<Protocols::ForeignToplevelManagement::GForeignToplevelManager*> &foreignToplevelManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [ext_foreign_toplevel_list_v1](https://wayland.app/protocols/ext-foreign-toplevel-list-v1#ext_foreign_toplevel_list_v1) global
     * of the Foreign Toplevel List protocol.
     */
    const std::vector<Protocols::ForeignToplevelList::GForeignToplevelList*> &foreignToplevelListGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_single_pixel_buffer_manager_v1](https://wayland.app/protocols/single-pixel-buffer-v1#wp_single_pixel_buffer_manager_v1) global
     * of the Single Pixel Buffer protocol.
     */
    const std::vector<Protocols::SinglePixelBuffer::GSinglePixelBufferManager*> &singlePixelBufferManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_content_type_manager_v1](https://wayland.app/protocols/content-type-v1#wp_content_type_manager_v1) global
     * of the Content Type Hint protocol.
     */
    const std::vector<Protocols::ContentType::GContentTypeManager*> &contentTypeManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [ext_idle_notifier_v1](https://wayland.app/protocols/ext-idle-notify-v1#ext_idle_notifier_v1) global
     * of the Idle Notify protocol.
     */
    const std::vector<Protocols::IdleNotify::GIdleNotifier*> &idleNotifierGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwp_idle_inhibit_manager_v1](https://wayland.app/protocols/idle-inhibit-unstable-v1#zwp_idle_inhibit_manager_v1) global
     * of the Idle Inhibit protocol.
     */
    const std::vector<Protocols::IdleInhibit::GIdleInhibitManager*> &idleInhibitManagerGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [xdg_activation_v1](https://wayland.app/protocols/xdg-activation-v1#xdg_activation_v1) global
     * of the XDG Activation protocol.
     */
    const std::vector<Protocols::XdgActivation::GXdgActivation*> &xdgActivationGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [wp_drm_lease_device_v1](https://wayland.app/protocols/drm-lease-v1#wp_drm_lease_device_v1) global
     * of the DRM Lease protocol.
     */
    const std::vector<Protocols::DRMLease::GDRMLeaseDevice*> &drmLeaseDeviceGlobals() const noexcept;

    /**
     * Resources created when the client binds to the
     * [zwlr_output_manager_v1](https://wayland.app/protocols/wlr-output-management-unstable-v1#zwlr_output_manager_v1) global
     * of the wlroots Output Management protocol.
     */
    const std::vector<Protocols::WlrOutputManagement::GWlrOutputManager*> &wlrOutputManagerGlobals() const noexcept;

    LPRIVATE_IMP_UNIQUE(LClient)
};

#endif // LCLIENT_H
