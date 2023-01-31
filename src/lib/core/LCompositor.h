#ifndef LCOMPOSITOR_H
#define LCOMPOSITOR_H

#include <libinput.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include <list>
#include <algorithm>

#include <LClient.h>
#include <LOutputManager.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDNDManager.h>
#include <LPopupRole.h>
#include <LSubsurfaceRole.h>
#include <LCursorRole.h>
#include <LDNDIconRole.h>

#include <LNamespaces.h>
#include <condition_variable>
#include <sys/eventfd.h>

#include <mutex>
#include <thread>

using namespace std;

/*!
 * @brief Main class and resources factory
 *
 * The LCompositor class is responsible for initializing the Wayland event loop and the input and graphic backend with the start() method.\n
 * The initialized() virtual method is called after start() to notify the correct initialization of the compositor.\n
 * Any initial configuration should be done within said method or later to avoid segmentation faults.
 *
 * LCompositor also follows the ***factory*** design pattern. It has virtual methods used
 * as virtual constructors and destructors of the many classes offered by the library. This allows the developer to override the virtual methods of those classes 
 * and change their default implementation.
 */
class Louvre::LCompositor
{
public:

    /*!
     * @brief Constructor of the LCompositor class.
     */
    LCompositor();

    /*!
     * @brief Destructor of the LCompositor class.
     */
    virtual ~LCompositor();

    LCompositor(const LCompositor&) = delete;
    LCompositor& operator= (const LCompositor&) = delete;

    bool graphicBackendInitialized() const;
    bool inputBackendInitialized() const;

    /*!
     * @brief Loads a graphic backend.
     *
     * Loads a graphic backend (dynamic library) used to display the compositor. If this method is not used, the library will use
     * the DRM backend by default located at ```/usr/etc/Louvre/backends/libLGraphicBackendDRM.so```.\n
     *
     * @param path Location of the backend's dynamic library.
     *
     * @returns True if the backend is successfully loaded.
     */
    bool loadGraphicBackend(const char *path);

    /*!
     * @brief Loads an input backend.
     *
     * Loads an input backend (dynamic library) to access the events of input devices. If this method is not used, the library will use
     * the Libinput backend by default located at ```/usr/etc/Louvre/backends/libLInputBackendLibinput.so```.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @returns True if the backend is loaded successfully.
     */
    bool loadInputBackend(const char *path);


    /*!
     * @brief Compositor's global scale.
     *
     * Returns the largest scale of the outputs added to the compositor. The value can change when adding or removing an output to the compositor or when any of the
     * added ones are assigned a scale with the LOutput::setScale() method. The compositor calls the virtual method globalScaleChanged() when its value changes.
     */
    Int32 globalScale() const;

    /*!
     * @brief Notifies a compositor's global scale change.
     *
     * Called when the global scale of the compositor changes. Reimplement this method to update all variables defined in compositor coordinates.
     * All classes with properties defined in compositor coordinates (terminated with the **C** suffix) automatically update their values.
     *
     * @param oldScale Old scale.
     * @param newScale New scale.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp globalScaleChanged
     */
    virtual void globalScaleChanged(Int32 oldScale, Int32 newScale);

    /*!
     * @brief Notifies the correct initialization of the compositor.
     *
     * Notifies the correct initialization of the Wayland event loop and backends after calling the start() method.\n
     * This method should be used to perform the initial configuration of outputs (monitors) as shown in the default implementation. 
     * The available outputs can be accessed from the LOutputManager class instance, accessible with outputManager().
     * If you find an available output you can add it to the compositor with the addOutput() method, this will initialize its context 
     * and rendering thread.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized();


    /*!
     * @brief Reports the successful initialization of the cursor.
     *
     * Report correct initialization of the cursor after the first output is added to the compositor.\n
     * Within this method it is recommended to load cursor textures you may want to use. The LCursor class offers
     * the LCursor::loadXCursorB() method for loading pixmaps of X cursors available on the system.\n
     * The default implementation shows a commented example on how to load X cursors pixmaps and assign them to the cursor.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp cursorInitialized
     */
    virtual void cursorInitialized();

    /*!
     * @name Virtual Constructors
     * Virtual constructors are used to dynamically instantiate variants of a base or abstract class.\n
     * The compositor uses virtual constructors to create and keep track of resources like LSurfaces, LOutputs, LSeats, etc, 
     * and for letting the developer use its own subclasses and override their virtual methods.
     */

///@{

    /*!
     * @brief Virtual constructor of the LOutputManager class.
     *
     * Called by the graphic backend.\n
     * The LOutputManager class has virtual methods that notify the hotplugging of GPU sockets.
     * For example, when connecting or disconnecting a monitor through a computer's HDMI port.\n
     * Must return an instance of LOutputManager or a subclass of LOutputManager.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createOutputManagerRequest
     */
    virtual LOutputManager *createOutputManagerRequest(LOutputManager::Params *params);

    /*!
      * @brief Virtual constructor of the LOutput class.
      *
      * Called by the graphic backend for each available output.\n
      * The LOutput class has virtual methods to initialize graphic contexts and do rendering.
      * See the LOutput::initializeGL() and LOutput::paintGL() methods for more information.\n
      * Must return an instance of LOutput or a subclass of LOutput.
      *
      * #### Default implementation
      * @snippet LCompositorDefault.cpp createOutputRequest
      */
    virtual LOutput *createOutputRequest();

    /*!
     * @brief Virtual constructor of the LClient class.
     *
     * Called when a new client establishes a connection with the compositor.\n
     * The LClient class does not have virtual methods, therefore its reimplementation is not necessary.
     * However you can use this method to know when a client connects to the compositor.\n
     * Must return an instance of LClient or subclass of LClient.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createClientRequest
     */
    virtual LClient *createClientRequest(LClient::Params *params);

    /*!
     * @brief Virtual constructor of the LSurface class
     *
     * Called when a client creates a new surface.\n
     * The LSurface class has many virtual methods to notify changes of its role, size, mapping, etc.\n
     * Must return an instance of LSurface or a subclass of LSurface.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSurfaceRequest
     */
    virtual LSurface *createSurfaceRequest(LSurface::Params *params);

    /*!
     * @brief Virtual constructor of the LSeat class.
     *
     * Called during the compositor initialization.\n
     * The LSeat class has virtual method to access all events generated by the input backend.\n
     * Must return an instance of LSeat or a subclass of LSeat.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSeatRequest
     */
    virtual LSeat *createSeatRequest(LSeat::Params *params);

    /*!
     * @brief Virtual constructor of the LPointer class.
     *
     * Called during LSeat initialization.\n
     * The LPointer class has virtual methods to access pointer events generated by the input backend.\n
     * Must return an instance of LPointer or a subclass of LPointer.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createPointerRequest
     */
    virtual LPointer *createPointerRequest(LPointer::Params *params);

    /*!
     * @brief Virtual constructor of the LKeyboard class.
     *
     * Called during LSeat initialization.\n
     * The LKeyboard class has virtual methods to access keyboard events generated by the input backend.\n
     * Must return an instance of LKeyboard or a subclass of LKeyboard.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createKeyboardRequest
     */
    virtual LKeyboard *createKeyboardRequest(LKeyboard::Params *params);

    /*!
     * @brief Virtual constructor of the LDNDManager class.
     *
     * Called during LSeat initialization.\n
     * The LDNDManager class has virtual methods that notify the start and end of Drag & Drop sessions between clients.\n
     * Must return an instance of LDNDManager or a subclass of LDNDManager.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createDNDManagerRequest
     */
    virtual LDNDManager *createDNDManagerRequest(LDNDManager::Params *params);

    /*!
     * @brief Virtual constructor of the LToplevelRole class.
     *
     * Called when a client creates a Toplevel role for a surface.\n
     * The LToplevelRole class has virtual methods that notify,
     * its geometry change, state change (maximized, minimized, etc), the start
     * of interactive moving and resizing sessions, and so on.\n
     * Must return an instance of LToplevelRole or a subclass of LToplevelRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createToplevelRoleRequest
     */
    virtual LToplevelRole *createToplevelRoleRequest(LToplevelRole::Params *params);

    /*!
     * @brief Virtual constructor of the LToplevelRole class.
     *
     * Called when a client creates a Popup role for a surface.\n
     * The LPopupRole class has virtual methods that notify,
     * its geometry change, its repositioning based on its LPositioner, and so on.\n
     * Must return an instance of LPopupRole or a subclass of LPopupRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createPopupRoleRequest
     */
    virtual LPopupRole *createPopupRoleRequest(LPopupRole::Params *params);

    /*!
     * @brief Virtual constructor of the LSubsurfaceRole class.
     *
     * Called when a client creates a Subsurface role for a surface.\n
     * The LSubsurfaceRole class has virtual methods that notify its local position change
     * relative to its parent, its rearrangement on the stack of neighboring surfaces, and more.\n
     * Must return an instance of LSubsurfaceRole or a subclass of LSubsurfaceRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSubsurfaceRoleRequest
     */
    virtual LSubsurfaceRole *createSubsurfaceRoleRequest(LSubsurfaceRole::Params *params);

    /*!
     * @brief Virtual constructor of the LCursorRole class.
     *
     * Called when a client wants to use a surface as a cursor.\n
     * The LCursorRole class has a virtual method that notifies the hotspot change
     * of the cursor being rendered.\n
     * Must return an instance of LCursorRole or a subclass of LCursorRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createCursorRoleRequest
     */
    virtual LCursorRole *createCursorRoleRequest(LCursorRole::Params *params);

    /*!
     * @brief Virtual constructor of the LDNDIconRole class.
     *
     * Called when a client wants to use a surface as an icon for a Drag & Drop session.\n
     * Similar to LCursorRole, the LDNDIconRole class has a virtual method that notifies the hotspot change
     * of the icon being dragged.\n
     * Must return an instance of LDNDIconRole or a subclass of LDNDIconRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createDNDIconRoleRequest
     */
    virtual LDNDIconRole *createDNDIconRoleRequest(LDNDIconRole::Params *params);

///@}

    /*!
     * @name Virtual Destructors
     * Virtual destructors are used by the compositor to provide early notification of the destruction of a resource.\n
     *
     * @warning The compositor internally handles the freeing of memory after the destructor is called, 
     * so the developer should not call the "delete" method on the resource provided in the argument. 
     * Only memory managed by the developer must be freed separately since its not freed by the library.
     */

///@{

    /*!
     * @brief Virtual destructor of the LOutput class.
     *
     * Invoked by the graphic backend when an output becomes unavailable.\n
     * The virtual method LOutputManager::unpluggedOutputRequest() and the removeOutput() 
     * method should be used to unlink the output from the compositor, stop its thread and render loop, 
     * and this virtual method to free any memory allocated in the LOutput reimplementation.
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyOutputRequest
     */
    virtual void destroyOutputRequest(LOutput *output);

    /*!
     * @brief Virtual destructor of the LClient class.
     *
     * Called when a client disconnects and after all its resources
     * have been released.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyClientRequest
     */
    virtual void destroyClientRequest(LClient *client);

    /*!
     * @brief Virtual destructor of the LSurface class.
     *
     * Called when a client requests to destroy one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroySurfaceRequest
     */
    virtual void destroySurfaceRequest(LSurface *surface);

    /*!
     * @brief Virtual destructor of the LToplevelRole class.
     *
     * Called when a client requests to destroy the Toplevel role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyToplevelRoleRequest
     */
    virtual void destroyToplevelRoleRequest(LToplevelRole *toplevel);

    /*!
     * @brief Virtual destructor of the LPopupRole class.
     *
     * Called when a client requests to destroy the Popup role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyPopupRoleRequest
     */
    virtual void destroyPopupRoleRequest(LPopupRole *popup);

    /*!
     * @brief Virtual destructor of the LCursorRole class.
     *
     * Invoked when a client requests to destroy the Cursor role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyCursorRoleRequest
     */
    virtual void destroyCursorRoleRequest(LCursorRole *cursor);

    /*!
     * @brief Virtual destructor of the LDNDIconRole class.
     *
     * Invoked when a client requests to destroy the DNDIcon role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyDNDIconRoleRequest
     */
    virtual void destroyDNDIconRoleRequest(LDNDIconRole *icon);

///@}

    /*!
     * @brief Starts the compositor.
     *
     * Starts the Wayland event loop and backends. After notifying its successful initialization with initialized(), the
     * compositor is able to receive connections from Wayland clients and initialize output rendering threads with the addOutput() method.\n
     * @returns EXIT_FAILURE on failure and EXIT_SUCCESS otherwise.
     */
    int start();

    /*!
     * @brief Ends the compositor process.
     */
    void finish();

    /*!
     * @brief Raises a surface.
     *
     * Places a surface at the end of the compositor's surfaces list (see surfaces()), which means it will be the last surface to be rendered in a frame, 
     * and therefore positioned in front of all others. If the surface being repositioned is a parent of other surfaces, these will also be elevated, 
     * maintaining the current hierarchical order required by certain protocols. Raising a surface that does not exist in the compositor's list, is a no-op.
     *
     * @param surface Surface to be raised.
     */
    void raiseSurface(LSurface *surface);

    /*!
     * @brief Compositor's cursor
     *
     * Returns a pointer to the cursor implementation provided by the library.\n
     * Must be accessed within or after the cursorInitialized() virtual method is invoked.\n
     * If the cursor has not yet been initialized, this method returns nullptr.
     */
    LCursor *cursor() const;

    /*!
     * @brief Compositor's seat
     *
     * Returns a pointer to the compositor seat from which the LPointer, LKeyboard and LTouch instances can be accessed.\n
     * Must be accessed after compositor initialization.
     */
    LSeat *seat() const;

    /*!
     * @brief Schedules the next rendering frame of all outputs.
     *
     * This function calls the LOutput::repaint() method of all outputs added to the compositor.\n
     * The order in which the outputs are rendered is undefined since each one works in its own thread.
     */
    void repaintAllOutputs();

    /*!
     * @brief Adds an output to the compositor and initializes it.
     *
     * Creates a new thread and render loop for the output and initializes it. Once initialized, the graphical backend invokes
     * the virtual method LOutput::initializeGL() to perform an initial setup and it is possible to call the
     * LOutput::repaint() method to schedule rendering frames.\n
     * The list of outputs added to the compositor can be accessed with outputs().\n
     * Add an already initialized output is a no-op.
     *
     * @param output One of the outputs available on the outputs manager (outputManager()) through the LOutputManager::outputs() method.
     */
    void addOutput(LOutput *output);

    /*!
     * @brief Removes an output from the compositor.
     *
     * Removes the output of the compositor, stopping its thread and rendering loop.
     *
     * @param output One of the outputs previously added to the compositor, accessible from outputs(). Removing an output that has not been added to the compositor is a no-op.
     */
    void removeOutput(LOutput *output);

    /*!
     * @brief Output manager.
     *
     * Returns the output manager created by the graphic backend.\n
     * The output manager provides access to the list of currently available outputs generated by the graphic backend.\n
     * Additionally, it has virtual methods that notify the connection and disconnection of these outputs, for example, 
     * when connecting or disconnecting an external monitor through an HDMI port.
     *
     * Must be accessed after compositor initialization.
     */
    LOutputManager *outputManager() const;

    /*!
     * @brief List of surfaces.
     *
     * List of all surfaces created by clients.\n
     * Use the LClient::surfaces() method to access surfaces created by a specific client.
     */
    const list<LSurface*>&surfaces() const;

    /*!
     * @brief List of initialized outputs.
     *
     * List of all initialized outputs with the addOutput() method.\n
     */
    const list<LOutput*>&outputs() const;

    /*!
      * @brief Clients list.
      *
      * List of clients connected to the compositor.
      */
    const list<LClient*>&clients() const;

    /*!
      * @brief Client from native Wayland resource.
      *
      * @returns The LClient instance for a **wl_client** resource or nullptr if not found.
      */
    LClient *getClientFromNativeResource(wl_client *client);

    /*!
      * @brief ID of the main thread.
      *
      * The main thread handles the Wayland and input backend event loop, while each output runs on its own thread.
      */
    std::thread::id mainThreadId() const;

    class LCompositorPrivate;

    /*!
     * @brief Access to the private API of LCompositor.
     *
     * Returns an instance of the LCompositorPrivate class (following the ***PImpl Idiom*** pattern) which contains all the private members of LCompositor.\n
     * Used internally by the library.
     */
    LCompositorPrivate *imp() const;

private:
    mutable LCompositorPrivate *m_imp = nullptr;
};

#endif // LCOMPOSITOR_H
