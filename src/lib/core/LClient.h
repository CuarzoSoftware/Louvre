#ifndef LCLIENT_H
#define LCLIENT_H

#include <LObject.h>
#include <LRegion.h>
#include <LSurface.h>
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
 * The LClient class represents a Wayland client connected to the compositor.
 * It allows managing client connections, accessing its resources created through various Wayland protocols,
 * handling ping/pong events, and more.
 */
class Louvre::LClient : public LObject
{
public:

    struct Params;

    // TODO add doc
    struct PointerEvents
    {
        LPointerEnterEvent enter;
        LPointerLeaveEvent leave;
        LPointerButtonEvent button[5];
        UInt8 buttonIndex { 0 };

        LPointerSwipeBeginEvent swipeBegin;
        LPointerSwipeEndEvent swipeEnd;
        LPointerPinchBeginEvent pinchBegin;
        LPointerPinchEndEvent pinchEnd;
        LPointerHoldBeginEvent holdBegin;
        LPointerHoldEndEvent holdEnd;
    };

    struct KeyboardEvents
    {
        LKeyboardEnterEvent enter;
        LKeyboardKeyEvent key[5];
        UInt8 keyIndex { 0 };
        LKeyboardLeaveEvent leave;
        LKeyboardModifiersEvent modifiers;
    };

    struct TouchEvents
    {
        std::vector<LTouchDownEvent> down;
        std::vector<LTouchUpEvent> up;
    };

    struct Events
    {
        PointerEvents pointer;
        KeyboardEvents keyboard;
        TouchEvents touch;
    };

    /**
     * @brief Constructor of the LClient class.
     *
     * @param params Internal library parameters passed in the LCompositor::createClientRequest() virtual constructor.
     */
    LClient(const void *params);

    /**
     * @brief Destructor of the LClient class.
     */
    ~LClient();

    /// @cond OMIT
    LClient(const LClient&) = delete;
    LClient& operator= (const LClient&) = delete;
    /// @endcond

    /**
     * @brief Sends a Ping event to the client for responsiveness detection.
     *
     * This method sends a Ping event to the client, which is expected to acknowledge it by invoking the `pong()` virtual method.
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
     * Reimplement this virtual method if you want to be notified when a client responds to a ping() event.
     *
     * @param serial The same serial number passed in ping().
     *
     * @par Default Implementation
     * @snippet LClientDefault.cpp pong
     */
    virtual void pong(UInt32 serial) noexcept;

    /**
     * @brief Native `wl_client` struct of the client.
     *
     * This method returns a pointer to the native `wl_client` struct associated with the client. The `wl_client` struct is part of the original Wayland library.
     *
     * @return A pointer to the native `wl_client` struct of the client.
     */
    wl_client *client() const noexcept;

    /**
     * @brief Immediately flushes pending events.
     *
     * Use this method to forcefully and immediately flush any pending Wayland client events.
     * It ensures that all pending events in the client's event queue are processed and handled without delay.
     */
    void flush() noexcept;

    /**
     * @brief Terminates the client connection with the compositor.
     *
     * This method terminates the client's connection with the compositor, which is equivalent to invoking `wl_client_destroy()`.
     * It effectively closes the connection between the client and the compositor.
     *
     * @note All resources created by the client are automatically destroyed in the reverse order in which they were created.
     */
    void destroy() noexcept;

    /**
     * Resources created when the client binds to the [wl_output](https://wayland.app/protocols/wayland#wl_output) global.\n
     * The library creates a [wl_output](https://wayland.app/protocols/wayland#wl_output)
     * global for each output added to the compositor in
     * order to notify the client the available outputs and their properties.
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

    // TODO
    const Events &events() const noexcept;
    const LEvent *findEventBySerial(UInt32 serial) const noexcept;
    const LClientCursor &lastCursorRequest() const noexcept;

    LPRIVATE_IMP_UNIQUE(LClient)
};

#endif // LCLIENT_H
