#ifndef LSEAT_H
#define LSEAT_H

#include <LObject.h>
#include <LToplevelRole.h>
#include <LSurface.h>

struct libseat;

/*!
 * @brief Group of input and output devices
 *
 * The LSeat class represents a compositor seat. A seat is a group of input and output devices such as mouse, keyboard,
 * a GPU, etc used in a session. Typically, permission to access these devices is limited to a single process per session,
 * which is usually a Wayland or X11 compositor.\n
 * To allow the compositor to control the devices in the seat without needing to be a superuser, the DRM and Libinput backends use the openDevice()
 * method to open the devices, which internally implements libseat to request seat permissions.\n
 * The LSeat class also allows for changing sessions (TTY), accessing the clipboard, accessing instances of LPointer and LKeyboard, listening to native input backend events,
 * among other functions.
 */
class Louvre::LSeat : public LObject
{
public:
    struct Params;

    /*!
     * @brief Input capabilities
     *
     * Compositor input capabilities.\n
     */
    enum InputCapabilities : UInt32
    {
        /// Pointer events
        Pointer = 1,

        /// Keyboard events
        Keyboard = 2,

        /// Touch events
        Touch = 4
    };

    /*!
     * @brief LSeat class constructor.
     *
     * There is only one instance of LSeat, which can be accessed from LCompositor::seat().
     *
     * @param params Internal library parameters provided in the virtual LCompositor::createSeatRequest() constructor.
     */
    LSeat(Params *params);

    /*!
     * @brief LSeat class destructor.
     *
     * Invoked after LCompositor::destroySeatRequest().
     */
    virtual ~LSeat();

    LSeat(const LSeat&) = delete;
    LSeat& operator= (const LSeat&) = delete;

    /*!
     * @brief Available outputs.
     *
     * List of currently available outputs.
     */
    const list<LOutput*>*outputs() const;

    /*!
     * @brief Handle to the native context used by the input backend.
     *
     * This opaque handle is unique to each input backend.\n
     * In the case of the Libinput backend, it is equivalent to a pointer to a [struct libinput](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput.html).\n
     * In the case of the X11 backend, it is equivalent to a pointer to a [struct Display](https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html).\n\n
     *
     * You can use this handle to configure specific aspects of each backend.
     */
    void *backendContextHandle() const;

    /*!
     * @brief Input Backend Capabilities
     *
     * Flags representing the input capabilities of the backend, defined in #Capabilities.
     */
    UInt32 backendCapabilities() const;

    /*!
     * Seat name (E.g. "seat0").
     */
    const char *name() const;

    /*!
     * @brief Input capabilities of the compositor.
     *
     * Flags with the input capabilities of the compositor assigned with setCapabilities().\n
     */
    UInt32 capabilities() const;

    /*!
     * @brief Assigns the input capabilities of the compositor.
     *
     * Notifies clients of the compositor's input capabilities.\n
     * Clients will only listen to events specified in the capabilities.\n
     * The default implementation of initialized() sets the compositor's capabilities to those of the input backend.\n
     *
     * @param capabilitiesFlags Flags with the input capabilities of the compositor defined in #Capabilities. They may differ from the input capabilities of the backend.
     */
    void setCapabilities(UInt32 capabilitiesFlags);

    /*!
     * @brief Active Toplevel surface.
     *
     * Pointer to the active LToplevelRole role assigned by passing the flag LToplevelRole::Active in LToplevelRole::confgureC().\n
     * Only one Toplevel surface can be active, the library automatically deactivates other Toplevels when one is activated.
     *
     * @returns nullptr if no Toplevel is active.
     */
    LToplevelRole *activeToplevel() const;

    /*!
     * @brief Access to pointer events.
     *
     * Access to the LPointer instance used to receive pointer events from the backend and redirect them to clients.
     */
    LPointer *pointer() const;

    /*!
     * @brief Access to keyboard events.
     *
     * Access to the LKeyboard instance used to receive keyboard events from the backend and redirect them to clients.
     */
    LKeyboard *keyboard() const;

    /*!
     * @brief Access to the drag & drop session manager.
     *
     * Access to the LDNDManager instance used to handle drag & drop sessions.
     */
    LDNDManager *dndManager() const;

    /*!
     * @brief Access to the clipboard.
     *
     * Access to the clipboard (data source) assigned by the last client.\n
     * @returns nullptr if the clipboard has not been assigned.
     */
    LDataSource *dataSelection() const;

/// @name Session
/// @{
    /*!
     * @brief Switch session.
     *
     * Switch session (TTY). The default implementation of LKeyboard::keyEvent() allows switching sessions
     * using the shortcuts \n```Ctrl + Alt + (F1, F2, ..., F10)```.
     * @param tty TTY number.
     *
     * @returns 0 if the session is successfully switched and -1 in case of error.
     */
    Int32 setTTY(Int32 tty);

    /*!
     * @brief Opens a device.
     *
     * Opens a device on the seat, returning its ID and storing its file descriptor in **fd**.\n
     * The DRM graphic backend and the Libinput input backend use this function to open GPUs and input devices respectively.
     * @param path Location of the device.
     * @param fd Stores the file descriptor.
     * @returns The ID of the device or -1 in case of an error.
     */
    Int32 openDevice(const char *path, Int32 *fd, Int32 flags = 0);

    /*!
     * @brief Closes a device.
     *
     * @param id The id returned by openDevice().
     * @returns 0 if the device is closed successfully and -1 in case of error.
     */
    Int32 closeDevice(Int32 id);

    /*!
     * @brief Libseat handle.
     *
     * [struct libseat](https://github.com/kennylevinsen/seatd/blob/master/include/libseat.h) handle.
     */
    libseat *libseatHandle() const;

    /*!
     * @brief State of the seat.
     *
     * @returns true if the seat is active and false otherwise.
     */
    bool enabled() const;

    LPopupRole *topmostPopup() const;

/// @}

/// @name Virtual Methods
/// @{
    /*!
     * @brief Seat initialization.
     *
     * Reimplement this virtual method if you want to be notified when the seat is initialized.\n
     * #### Default implementation
     * @snippet LSeatDefault.cpp initialized
     */
    virtual void initialized();

    /*!
     * @brief Request to set the clipboard.
     *
     * Reimplement this virtual method if you want to control which clients can set the clipboard.\n
     * The default implementation allows clients to set the clipboard only if one of their surfaces has pointer or keyboard focus.\n
     *
     * Returning true grants the client permission to set the clipboard and false prohibits it.\n
     *
     * @param device Data device that makes the request.
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp setSelectionRequest
     */
    virtual bool setSelectionRequest(LDataDevice *device, UInt32 serial);

    /*!
     * @brief Native input backend events
     *
     * Reimplement this virtual method if you want to access all events generated by the input backend.\n
     *
     * @param event Opaque handle to the native backend event. In the Libinput backend it corresponds to a [libinput_event](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput__event.html) struct 
     * and in the X11 backend to a [XEvent](https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html) struct.
     *
     * #### Default implementation
     * @snippet LSeatDefault.cpp backendNativeEvent
     */
    virtual void backendNativeEvent(void *event);

    /*!
     * @brief Enabled seat
     *
     * Reimplement this virtual method if you want to be notified when the seat is enabled.\n
     * The seat is enabled when the session (TTY) in which the compositor is running is resumed.
     *
     * #### Default implementation
     * @snippet LSeatDefault.cpp seatEnabled
     */
    virtual void seatEnabled();

    /*!
     * @brief Disabled seat
     *
     * Reimplement this virtual method if you want to be notified when the seat is disabled.\n
     * The seat is disabled when switching to a different session (TTY) with setTTY().
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp seatEnabled
     */
    virtual void seatDisabled();

    /*!
     * @brief New available output.
     *
     * The outputPlugged() method is invoked by the graphic backend when a new output is available, for example when connecting an external monitor through a VGA or HDMI port.\n
     * You can reimplement this method to be notified when a new output is available.\n
     * The default implementation initializes the new output and positions it at the end of the already initialized outputs.
     *
     * ### Default Implementation
     * @snippet LSeatDefault.cpp outputPlugged
     */
    virtual void outputPlugged(LOutput *output);

    /*!
     * @brief Disconnected output.
     *
     * The outputUnplugged() method is invoked by the graphical backend when an output is no longer available, for example when an external monitor connected to a VGA or HDMI port is disconnected.\n
     * You can override this method to be notified when an output is no longer available.\n
     *
     * The default implementation removes the output from the compositor if it is initialized
     * and re-arranges the ones already initialized.
     *
     * #### Default Implementation
     * @snippet LSeatDefault.cpp outputUnplugged
     */
    virtual void outputUnplugged(LOutput *output);

/// @}

    LPRIVATE_IMP(LSeat)
};

#endif // LSEAT_H
