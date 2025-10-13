#ifndef LCOMPOSITOR_H
#define LCOMPOSITOR_H

#include <CZ/Louvre/Louvre.h>
#include <CZ/Louvre/LFactoryObject.h>
#include <CZ/Louvre/Layout/LSurfaceLayer.h>
#include <CZ/Core/CZLogger.h>
#include <CZ/Core/CZWeak.h>

#include <filesystem>
#include <thread>
#include <unordered_set>
#include <vector>
#include <list>

/**
 * @brief Louvre's core and objects factory.
 *
 * The LCompositor class manages the main event loop, backend systems, and provides virtual constructors
 * and destructors for Louvre objects with interfaces you can override.
 *
 * @see LFactoryObject
 * @see createObjectRequest()
 * @see onAnticipatedObjectDestruction()
 */
class CZ::LCompositor : public CZObject
{
public:

    struct
    {
        CZWeak<LGlobal> WlCompositor;
        CZWeak<LGlobal> WlSubcompositor;
        CZWeak<LGlobal> WlDRM;
        CZWeak<LGlobal> WlSeat;
        CZWeak<LGlobal> WlDataDeviceManager;

        CZWeak<LGlobal> Viewporter;
        CZWeak<LGlobal> Presentation;
        CZWeak<LGlobal> SessionLockManager;
        CZWeak<LGlobal> CursorShapeManager;
        CZWeak<LGlobal> SinglePixelBufferManager;
        CZWeak<LGlobal> ContentTypeManager;
        CZWeak<LGlobal> TearingControlManager;
        CZWeak<LGlobal> FractionalScaleManager;
        CZWeak<LGlobal> LinuxDMABuf;
        CZWeak<LGlobal> DRMSyncObjManager;
        CZWeak<LGlobal> IdleNotifier;
        CZWeak<LGlobal> IdleInhibitManager;
        CZWeak<LGlobal> RelativePointerManager;
        CZWeak<LGlobal> PointerGestures;
        CZWeak<LGlobal> PointerConstraints;
        CZWeak<LGlobal> ForeignToplevelList;
        CZWeak<LGlobal> ForeignToplevelImageCaptureSourceManager;
        CZWeak<LGlobal> OutputImageCaptureSourceManager;

        CZWeak<LGlobal> XdgWmBase;
        CZWeak<LGlobal> XdgDecorationManager;
        CZWeak<LGlobal> XdgOutputManager;
        CZWeak<LGlobal> XdgActivation;

        CZWeak<LGlobal> LvrInvisibleRegionManager;
        CZWeak<LGlobal> LvrBackgroundBlurManager;
        CZWeak<LGlobal> LvrSvgPathManager;

        CZWeak<LGlobal> WlrOutputManager;
        CZWeak<LGlobal> WlrGammaControlManager;
        CZWeak<LGlobal> WlrLayerShell;
        CZWeak<LGlobal> WlrForeignToplevelManager;
        CZWeak<LGlobal> WlrScreenCopyManager;

    } wellKnownGlobals;

    /**
     * @brief Compositor state.
     *
     * @see state().
     */
    enum CompositorState
    {
        /// Uninitialized
        Uninitialized,

        /// Changing from uninitialized to initialized
        Initializing,

        /// Initialized
        Initialized,

        /// Changing from any state to uninitialized
        Uninitializing,

        /// Changing from initialized to paused
        Pausing,

        /// Idle state during a session (TTY) switch
        Paused,

        /// Changing from paused to initialized
        Resuming
    };

    /**
     * @brief Constructor of the LCompositor class.
     */
    LCompositor() noexcept;

    /**
     * @brief Destructor of the LCompositor class.
     */
    virtual ~LCompositor() noexcept;

    /**
     * @brief Gets the current compositor state.
     */
    CompositorState state() const noexcept;

    /**
     * @brief Sets a custom backend.
     *
     * Must be called before `start()`, otherwise it will be rejected. Pass `nullptr` to restore defaults.
     *
     * @note When a custom backend is set, Louvre won't fall back to others on failure.
     *
     * @return `true` if successful, `false` otherwise.
     */
    bool setBackend(std::shared_ptr<LBackend> backend) noexcept;

    /**
     * @brief Returns the currently set backend.
     *
     * @return Pointer to the current backend, or `nullptr` if none is set.
     */
    LBackend *backend() const noexcept;

    /**
     * @brief Starts the main event loop and backends.
     *
     * Triggers the initialized() event on success.
     *
     * @return `false` on failure and `true` otherwise.
     */
    bool start();

    /**
     * @brief Process the compositor's main event loop.
     *
     * @note You can also get a pollable file descriptor with fd().
     *
     * @param msTimeout Setting it to 0 disables the timeout, and setting it to -1 makes it wait indefinitely until an event occurs.
     *
     * @return 1 on success and 0 on failure.
     */
    int dispatch(int msTimeout);

    /**
     * @brief Gets a pollable file descriptor of the main event loop.
     */
    int fd() const noexcept;

    /**
     * @brief Uninitializes the compositor.
     *
     * @see uninitialized().
     */
    void finish() noexcept;

    /**
     * @brief Gets the native `wl_display` created by the compositor.
     */
    static wl_display *display() noexcept;

    /**
     * @brief Gets the compositor cursor.
     *
     * This method must be accessed within or after the initialized() event.
     * If the cursor has not yet been initialized, this method returns `nullptr`.
     *
     * @return A pointer to the LCursor instance or `nullptr` if not yet initialized.
     */
    LCursor *cursor() const noexcept;

    /**
     * @brief Gets the compositor seat.
     *
     * The seat provides access to the LPointer, LKeyboard, LTouch, and LOutput instances.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() const noexcept;

    /**
     * @brief Provides access to the session lock manager.
     *
     * The session lock manager allows you to handle client requests to lock the user session and display arbitrary content.
     */
    LSessionLockManager *sessionLockManager() const noexcept;

    /**
     * @brief Provides access to the activation token manager.
     *
     * The activation token manager allows you to handle client requests to activate surfaces of other clients.
     */
    LActivationTokenManager *activationTokenManager() const noexcept;

    /**
     * @brief List of all surfaces created by clients.
     *
     * @note Since Louvre v3, this list no longer reflects the correct stacking order required by protocols.
     *
     * @see layers()
     */
    const std::list<LSurface*> &surfaces() const noexcept;

    /**
     * @brief Returns the list of surfaces assigned to each layer.
     *
     * Each index in the array corresponds to a specific layer, as defined by the LSurfaceLayer enum.
     * Surfaces are grouped into these layers based on their assigned roles.
     *
     * @see LSurfaceLayer
     *
     * @example compositor()->layers()[LLayerMiddle]
     */
    const std::array<std::list<LSurface*>, LLayerOverlay + 1> &layers() const noexcept;

    /**
     * @brief List of clients connected to the compositor.
     */
    const std::vector<LClient*> &clients() const noexcept;

    /**
     * @brief Flush all pending client events.
     *
     * This method immediatly flushes all pending client events and configurations.
     */
    static void flushClients() noexcept;

    /**
      * @brief Gets the LClient of a native Wayland `wl_client` resource.
      *
      * @returns The LClient instance for a `wl_client` resource or `nullptr` if not found.
      */
    LClient *getClientFromNativeResource(const wl_client *client) noexcept;

    /**
     * @brief Gets a vector of all initialized outputs.
     *
     * This method returns a vector of all outputs that have been initialized using the addOutput() method.
     *
     * @note This vector only contains initialized outputs. To get all available outputs, use LSeat::outputs() instead.
     */
    const std::vector<LOutput*> &outputs() const noexcept;

    /**
     * @brief Initializes the specified output.
     *
     * This method creates a new rendering thread and initializes the specified output. Once initialized,
     * the LOutput::initializeGL() event is triggered, and LOutput::repaint() can be used to unlock the
     * rendering thread loop, which triggers LOutput::paintGL() events.
     *
     * This adds the output to the outputs() vector.
     *
     * @note Adding an already initialized output is a no-op.
     *
     * @param output The output to initialize, obtained from LSeat::outputs().
     *
     * @return `true` on success, `false` on failure.
     */
    bool addOutput(LOutput *output);

    /**
     * @brief Uninitializes the specified output.
     *
     * This method uninitializes and removes the specified output from the compositor, stopping its thread, rendering loop
     * and triggering the LOutput::uninitializeGL() event.
     *
     * @note Removing an output that has not been added to the compositor is a no-op.
     *
     * @warning Calling this method from the same rendering thread of the output (whithin any of its `...GL()` events) has no effect.
     *          Doing so would lead to a deadlock, so Louvre simply ignores the request.
     *
     * @param output The output to remove, previously added to the compositor. See outputs().
     */
    void removeOutput(LOutput *output);

    /**
     * @brief Unlocks the rendering thread of all initialized outputs.
     *
     * This method schedules a new rendering frame by calling the LOutput::repaint() method to all initialized outputs.
     */
    void repaintAllOutputs() noexcept;

    /**
     * @brief Searches for the most intersected output
     *
     * Checks which output within the global compositor space intersects the given rectangle the most.
     *
     * @param rect The rectangle to check in compositor-global coordinates.
     * @param initializedOnly If `true`, only considers initialized outputs. Default is `true`.
     * @return A pointer to the most intersected LOutput, or `nullptr` if no output is found.
     */
    LOutput *mostIntersectedOutput(const SkIRect &rect, bool initializedOnly = true) const noexcept;

    /**
     * @brief Identifier of the main thread.
     *
     * This ID corresponds to the primary thread responsible for managing the Wayland and input backend event loops,
     * while individual output operations are performed on separate threads.
     *
     * @return The identifier of the main thread.
     */
    std::thread::id mainThreadId() const noexcept;

    /**
     * @brief Creates and adds a global to the compositor.
     *
     * This method creates and adds a global to the compositor. Globals implemented by Louvre can be found in `<Louvre/Protocols/{protocol}/G{global}.h>`.
     *
     * Louvre automatically creates all the supported globals during compositor initialization. See LCompositor::createGlobalsRequest().
     *
     * @tparam Global The type of global to create. It should be a subclass of CZ::LResource.
     * @param version The maximum supported interface version, constrained by Louvre’s implemented version.
     *                If <= 0, Louvre’s highest supported version is used instead.
     * @param data User data to assign to the `wl_resource`.
     * @return Pointer to the created LGlobal instance, which can be later removed with removeGlobal().
     */
    template<class Global>
    LGlobal *createGlobal(Int32 version = 0, void *data = nullptr) noexcept
    {
        static_assert(std::is_base_of_v<CZ::LResource, Global> == true);

        CZWeak<LGlobal> *slot {};

        if (!Global::Probe(&slot))
            return nullptr;

        auto *global = globalCreate(Global::Interface(),
                            version <= 0 ? Global::MaxVersion() : std::min(version, Global::MaxVersion()),
                            data,
                            &Global::Bind);

        if (slot)
            *slot = global;

        return global;
    }

    /**
     * @brief Safely removes a global.
     *
     * This method performs a lazy removal of an LGlobal created with createGlobal(), ensuring clients are notified before it is destroyed.
     *
     * @note Calling LCompositor::removeGlobal() isn't mandatory as globals are automatically destroyed during the compositor's uninitialization.
     *
     * @warning After calling this method, the LGlobal instance must no longer be used as it is going to be destroyed.
     *
     * @param global Pointer to the LGlobal instance to remove.
     */
    void removeGlobal(LGlobal *global) noexcept;

    /**
     * @name Posix Signals
     */

    ///@{

    /**
     * @brief Registers a POSIX signal for synchronous handling.
     *
     * This method registers POSIX signals for synchronous dispatching by the main event loop,
     * as explained in [wl_event_loop_add_signal](https://wayland.freedesktop.org/docs/html/apc.html#Server-structwl__event__source_1a1706e2490502a95f24ccb59cbae3e2f8).
     *
     * All registered signals can be handled in onPosixSignal().
     *
     * Louvre automatically disables the signal in all rendering threads, but this does not apply to user-created threads.
     *
     * @note If called from a rendering thread, the signal addition will be scheduled for the next main loop iteration.
     *
     * @see removePosixSignal()
     * @see posixSignals()
     *
     * @return `true` if successfully added, `false` if the signal is already registered or
     *         if attempted while the compositor is uninitialized.
     */
    bool addPosixSignal(int signal) noexcept;

    /**
     * @brief Removes a registered POSIX signal.
     *
     * Unregisters and unblocks a previously added POSIX signal, preventing it from being handled by the main event loop.
     *
     * @note If called from a rendering thread, the signal removal will be scheduled for the next main loop iteration.
     *
     * @note Signals are automatically removed after the compositor is uninitialized.
     *
     * @return `true` if the signal was successfully removed, `false` if it was not registered.
     */
    bool removePosixSignal(int signal) noexcept;

    /**
     * @brief Retrieves the set of registered POSIX signals.
     *
     * Returns the set of POSIX signals currently registered via addPosixSignal().
     */
    const std::unordered_set<int> &posixSignals() const noexcept;

    ///@}

    /**
     * @name Virtual Methods
     */

    ///@{

    /**
     * @brief Notifies a successful compositor initialization.
     *
     * Use this event to handle the successful initialization of the compositor after calling start().\n
     * Here you should perform initial configuration tasks, such as setting up outputs, as demonstrated in the default implementation.
     *
     * @warning Avoid interacting with any LFactoryObjects (e.g., LPointer, LKeyboard, LTouch, etc.)
     *          or creating LTextures before this event. These objects or the input/graphic backends
     *          might be partially initialized, and interacting with them could cause a segmentation fault.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized() noexcept;

    // TODO: Add doc
    virtual void loadCursorShapes() noexcept;

    /**
     * @brief Notifies the uninitialization of the compositor.
     *
     * This event is triggered right before the compositor is uninitialized when finish() is called. At this point, both the input and graphic
     * backends, along with other resources such as connected clients and initialized outputs, are still operational.\n
     * Use this opportunity to release any compositor-specific resources that you may have created.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp uninitialized
     */
    virtual void uninitialized() noexcept;

    /**
     * @brief Request to create a specific LFactoryObject.
     *
     * Certain classes in Louvre have an interface that can be overridden.\n
     * Instances of these classes can only be created from this virtual constructor when requested by the compositor.\n
     * Refer to @ref LFactoryObject::Type to see all the classes that can be overridden.
     *
     * @param objectType Indicates the type of the class instance to return.
     * @param params An opaque data type that should be passed to the object's constructor.
     *
     * @return If `nullptr` is returned, Louvre will create an instance of the object using the base class.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createObjectRequest
     */
    virtual LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params);

    /**
     * @brief Announce the anticipated destruction of an LFactoryObject.
     *
     * When an LFactoryObject destructor is invoked, it is likely that associated resources are no longer available.\n
     * Using this event, however, provides an opportunity to access those resources.
     *
     * @warning Do not delete the object, Louvre deletes it later.
     *
     * @param object The object for which destruction is anticipated.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp onAnticipatedObjectDestruction
     */
    virtual void onAnticipatedObjectDestruction(LFactoryObject *object);

    /**
     * @brief Wayland globals initialization.
     *
     * Override this method if you want to remove or add custom Wayland globals.
     *
     * @see createGlobal()
     * @see removeGlobal()
     *
     * @return `true` on success, `false` on failure (prevents the compositor from starting).
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createGlobalsRequest
     */
    virtual bool createGlobalsRequest();

    /**
     * @brief Globals filter.
     *
     * Filter to prevent clients from using specific protocol globals.\n
     * It is recommended to use this method to allow only well-known clients to bind to
     * protocols such as Session Lock, Wlr Screen Copy, and others.
     *
     * See LClient::credentials() and LGlobal::isType().
     *
     * @param client The client attempting to bind to the global.
     * @param global The protocol global the client is attempting to bind to.
     * @returns `true` to allow the client, `false` to deny it.
     *
     * @par Default Implementation
     *
     * The default implementation allows all clients to bind to all globals.
     *
     * @snippet LCompositorDefault.cpp globalsFilter
     */
    virtual bool globalsFilter(LClient *client, LGlobal *global);

    /**
     * @brief Handles POSIX signals.
     *
     * Centralized function for managing all POSIX signals registered via addPosixSignal().
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp onPosixSignal
     */
    virtual void onPosixSignal(int signal);

    bool event(const CZEvent &e) noexcept override;

    CZLogger log;

    ///@}

    LPRIVATE_IMP_UNIQUE(LCompositor)

    LGlobal *globalCreate(const wl_interface *interface, Int32 version, void *data, wl_global_bind_func_t bind);
};

#endif // LCOMPOSITOR_H
