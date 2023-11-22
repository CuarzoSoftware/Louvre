#ifndef LCLIENT_H
#define LCLIENT_H

#include <LObject.h>
#include <LRegion.h>
#include <LSurface.h>

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

    /**
     * @brief Constructor of the LClient class.
     *
     * @param params Internal library parameters passed in the LCompositor::createClientRequest() virtual constructor.
     */
    LClient(Params *params);

    /**
     * @brief Destructor of the LClient class.
     */
    virtual ~LClient();

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
    bool ping(UInt32 serial) const;

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
    virtual void pong(UInt32 serial);

    /**
     * @brief Native `wl_client` struct of the client.
     *
     * This method returns a pointer to the native `wl_client` struct associated with the client. The `wl_client` struct is part of the original Wayland library.
     *
     * @return A pointer to the native `wl_client` struct of the client.
     */
    wl_client *client() const;

    /**
     * @brief Get the LDataDevice class for clipboard and drag & drop operations.
     *
     * The LDataDevice class is a wrapper for the [wl_data_device](https://wayland.app/protocols/wayland#wl_data_device) interface of the Wayland protocol. It is used for managing clipboard operations and facilitating drag & drop sessions.
     *
     * @return A reference to the LDataDevice class for clipboard and drag & drop operations.
     */
    LDataDevice &dataDevice() const;

    /**
     * @brief Get a list of surfaces created by the client.
     *
     * This method returns a constant reference to a list of LSurface pointers, representing the surfaces created by the client.
     *
     * @return A constant reference to the list of surfaces created by the client.
     */
    const std::list<LSurface*>& surfaces() const;

    /**
     * @brief Immediately flushes pending events.
     *
     * Use this method to forcefully and immediately flush any pending Wayland client events.
     * It ensures that all pending events in the client's event queue are processed and handled without delay.
     */
    void flush();

    /**
     * @brief Terminates the client connection with the compositor.
     *
     * This method terminates the client's connection with the compositor, which is equivalent to invoking `wl_client_destroy()`.
     * It effectively closes the connection between the client and the compositor.
     *
     * @note All resources created by the client are automatically destroyed in the reverse order in which they were created.
     */
    void destroy();

    /**
     * Returns a list of [wl_output](https://wayland.app/protocols/wayland#wl_output)
     * resources created when the client binds this global.\n
     * The library creates a [wl_output](https://wayland.app/protocols/wayland#wl_output)
     * global for each output added to the compositor in
     * order to notify the client the available outputs and their properties.
     */
    const std::list<Protocols::Wayland::GOutput*>&outputGlobals() const;

    /**
     * Resource created when the client binds to
     * the [wl_compositor](https://wayland.app/protocols/wayland#wl_compositor)
     * singleton global of the Wayland protocol.
     */
    const std::list<Protocols::Wayland::GCompositor*>&compositorGlobals() const;

    /**
     * List of resources created when the client binds to
     * the [wl_subcompositor](https://wayland.app/protocols/wayland#wl_subcompositor)
     * global of the Wayland protocol.
     */
    const std::list<Protocols::Wayland::GSubcompositor*>&subcompositorGlobals() const;

    /**
     * List of resources created when the client binds to
     * the [wl_seat](https://wayland.app/protocols/wayland#wl_seat)
     * global of the Wayland protocol.
     */
    const std::list<Protocols::Wayland::GSeat*>&seatGlobals() const;

    /**
     * Resource created when the client binds to
     * the [wl_data_device_manager](https://wayland.app/protocols/wayland#wl_data_device_manager)
     * singleton global of the Wayland protocol.
     */
    const Protocols::Wayland::GDataDeviceManager* dataDeviceManagerGlobal() const;

    /**
     * List of resources created when the client binds to the
     * [xdg_wm_base](https://wayland.app/protocols/xdg-shell#xdg_wm_base) global of the XdgShell protocol.
     */
    const std::list<Protocols::XdgShell::GXdgWmBase *> &xdgWmBaseGlobals() const;

    /**
     * List of resources created when the client binds to the
     * [zxdg_decoration_manager_v1](https://wayland.app/protocols/xdg-decoration-unstable-v1#zxdg_decoration_manager_v1) global
     * of the XdgDecoration protocol.
     *
     * The [zxdg_decoration_manager_v1](https://wayland.app/protocols/xdg-decoration-unstable-v1#zxdg_decoration_manager_v1)
     * interface allows the client and the compositor negotiate who should draw the decoration of toplevel surfaces.
     */
    const std::list<Protocols::XdgDecoration::GXdgDecorationManager*> &xdgDecorationManagerGlobals() const;

    /**
     * List of resources created when the client binds to the
     * [wp_presentation](https://wayland.app/protocols/presentation-time#wp_presentation) global of the
     * PresentationTime protocol.
     */
    const std::list<Protocols::WpPresentationTime::GWpPresentation*> &wpPresentationTimeGlobals() const;

    /**
     * List of resources generated when the client binds to the
     * [zwp_linux_dmabuf_v1](https://wayland.app/protocols/linux-dmabuf-unstable-v1#zwp_linux_dmabuf_v1) global
     * of the LinuxDMA-BUF protocol.
     */
    const std::list<Protocols::LinuxDMABuf::GLinuxDMABuf*> &linuxDMABufGlobals() const;

    LPRIVATE_IMP_UNIQUE(LClient)
};

#endif // LCLIENT_H
