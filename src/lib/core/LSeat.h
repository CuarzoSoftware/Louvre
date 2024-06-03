#ifndef LSEAT_H
#define LSEAT_H

#include <LFactoryObject.h>
#include <LToplevelRole.h>
#include <LSurface.h>

struct libseat;

/**
 * @brief Group of input and output devices.
 *
 * The LSeat class represents a collection of input and output devices such as a mouse, keyboard, touch screen,
 * and displays (outputs). These devices are used within a session. Typically, for security reasons, access to these devices is restricted to a single process per session,
 * often a Wayland compositor or X11 server.
 *
 * To enable the libinput and DRM backends to manage devices without requiring superuser privileges, the openDevice()
 * method is used to obtain device file descriptors. This method internally employs [libseat](https://github.com/kennylevinsen/seatd).
 *
 * By setting the environment variable **LOUVRE_ENABLE_LIBSEAT** to 0, [libseat](https://github.com/kennylevinsen/seatd) can be disabled, causing the compositor to launch without multi-session support.
 * Consequently, certain features such as switching to another TTY may become unavailable.\n
 *
 * @note Disabling [libseat](https://github.com/kennylevinsen/seatd) may be necessary in scenarios where the compositor needs to be started remotely via SSH.
 */
class Louvre::LSeat : public LFactoryObject
{
public:

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LSeat;

    /**
     * @brief Constructor of the LSeat class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LSeat(const void *params);

    /**
     * @brief Destructor of the LSeat class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LSeat();

    LCLASS_NO_COPY(LSeat)

    /**
     * @brief Vector of available outputs.
     *
     * This method provides a vector of currently available outputs. The vector includes connected outputs that can be initialized
     * as well as those that are already initialized.\n
     * To obtain only initialized outputs, refer to LCompositor::outputs().
     */
    const std::vector<LOutput *> &outputs() const noexcept;

    /**
     * @brief The seat name
     *
     * This method returns the name of the seat (E.g. "seat0").
     */
    const char *name() const noexcept;

    /**
     * @brief Active toplevel surface.
     *
     * This method returns the current LToplevelRole being configured with the @ref LToplevelRole::State::Activated flag.\n
     * Only one toplevel surface can be active at a time, Louvre automatically deactivates other toplevels when one is activated.
     *
     * @return The currently active LToplevelRole instance or `nullptr` if no toplevel is active.
     */
    LToplevelRole *activeToplevel() const noexcept;


    /**
     * @brief Active LToplevelRole resize sessions.
     *
     * @see LToplevelResizeSession and LToplevelRole::startResizeRequest()
     */
    const std::vector<LToplevelResizeSession*> &toplevelResizeSessions() const noexcept;


    /**
     * @brief Active LToplevelRole move sessions.
     *
     * @see LToplevelMoveSession and LToplevelRole::startMoveRequest()
     */
    const std::vector<LToplevelMoveSession*> &toplevelMoveSessions() const noexcept;

    /**
     * @brief Access to pointer events.
     *
     * Access to the LPointer instance used to receive pointer events from the backend and redirect them to clients.
     */
    LPointer *pointer() const noexcept
    {
        return m_pointer;
    }

    /**
     * @brief Access to keyboard events.
     *
     * Access to the LKeyboard instance used to receive keyboard events from the backend and redirect them to clients.
     */
    LKeyboard *keyboard() const noexcept
    {
        return m_keyboard;
    }

    /**
     * @brief Access to touch events.
     *
     * Access to the LTouch instance used to receive touch events from the backend and redirect them to clients.
     */
    LTouch *touch() const noexcept
    {
        return m_touch;
    }

    /**
     * @brief Access to the drag & drop session manager.
     */
    LDND *dnd() const noexcept
    {
        return m_dnd;
    }

    /**
     * @brief Access to the clipboard manager.
     */
    LClipboard *clipboard() const noexcept
    {
        return m_clipboard;
    }

    /**
     * @brief Close all popups.
     *
     * This method closes all active LPopupRole surfaces in reverse order of creation.
     *
     * @see LPopupRole::dismiss()
     */
    void dismissPopups() noexcept;

    /**
     * @brief Retrieve the topmost popup role.
     *
     * This method returns a pointer to the topmost popup.
     *
     * @return A pointer to the topmost LPopupRole or `nullptr` if there is no popup.
     */
    LPopupRole *topmostPopup() const noexcept;

/// @name Session
/// @{
    /**
     * @brief Switches session.
     *
     * This method allows you to switch to another session (TTY). Louvre also allows switching sessions
     * using the shortcuts ```Ctrl + Alt + [F1, F2, ..., F10]```.
     *
     * @param tty TTY number.
     */
    void setTTY(UInt32 tty) noexcept;

    /**
     * @brief Open a device.
     *
     * Opens a device returning its ID and storing its file descriptor in **fd**.\n
     * The DRM and Libinput backends use this method to open GPUs and input devices respectively.
     *
     * @param path Location of the device (E.g. "/dev/dri/card0")
     * @param fd Stores the file descriptor.
     * @returns The ID of the device or -1 in case of an error.
     */
    Int32 openDevice(const char *path, Int32 *fd) noexcept;

    /**
     * @brief Closes a device.
     *
     * This method is used by the input and graphic backends to close devices.
     *
     * @param id The id of the device returned by openDevice().
     * @returns 0 if the device is closed successfully and -1 in case of error.
     */
    Int32 closeDevice(Int32 id) noexcept;

    /**
     * @brief Libseat handle.
     *
     * [struct libseat](https://github.com/kennylevinsen/seatd/blob/master/include/libseat.h) handle.
     *
     * @note Returns `nullptr` if libseat is not being used.
     */
    libseat *libseatHandle() const noexcept;

    /**
     * @brief Check the session state.
     *
     * The session is considered disabled if the user is engaged in another active session (TTY).
     *
     * @return `true` if the seat is active, `false` otherwise.
     */
    bool enabled() const noexcept;

/// @}

/// @name Virtual Methods
/// @{

    /**
     * @brief Native input backend events
     *
     * Override this virtual method if you want to access all events generated by the input backend.
     *
     * @param event Opaque handle to the native backend event.
     *              If using Libinput backend it corresponds to a [libinput_event](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput__event.html) struct
     *
     * #### Default implementation
     * @snippet LSeatDefault.cpp nativeInputEvent
     */
    virtual void nativeInputEvent(void *event);

    /**
     * @brief Notify a change in the enabled() property
     *
     * Override this virtual method if you want to be notified when the seat is enabled or disabled.\n
     * The seat is enabled when the user is in the session (TTY) in which the compositor is running,
     * and disabled when switching to a different session.
     *
     * @see setTTY()
     *
     * #### Default implementation
     * @snippet LSeatDefault.cpp enabledChanged
     */
    virtual void enabledChanged();

    /**
     * @brief New available output.
     *
     * This event is invoked by the graphic backend when a new output is available, for example when connecting an external monitor through an HDMI port.\n
     * The default implementation initializes the new output and positions it at the end (right) of the already initialized outputs.
     *
     * ### Default Implementation
     * @snippet LSeatDefault.cpp outputPlugged
     */
    virtual void outputPlugged(LOutput *output);

    /**
     * @brief Disconnected output.
     *
     * This event is invoked by the graphic backend when an output is no longer available, for example when an
     * external monitor connected to an HDMI port is disconnected.
     *
     * The default implementation removes the output from the compositor and re-arranges the ones already initialized.
     *
     * @note Louvre automatically employs LCompositor::removeOutput() after this event, so calling it directly is not mandatory.
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp outputUnplugged
     */
    virtual void outputUnplugged(LOutput *output);

    /**
     * @brief New available input device.
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp inputDevicePlugged
     */
    virtual void inputDevicePlugged(LInputDevice *device);

    /**
     * @brief Disconnected input device.
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp inputDeviceUnplugged
     */
    virtual void inputDeviceUnplugged(LInputDevice *device);

/// @}

    LPRIVATE_IMP_UNIQUE(LSeat)
    friend class LCompositor;
    LPointer *m_pointer { nullptr };
    LKeyboard *m_keyboard { nullptr };
    LTouch *m_touch { nullptr };
    LDND *m_dnd { nullptr };
    LClipboard *m_clipboard { nullptr };
};

#endif // LSEAT_H
