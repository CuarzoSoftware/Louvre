#ifndef LTOPLEVELROLE_H
#define LTOPLEVELROLE_H

#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>
#include <CZ/Louvre/LMargins.h>
#include <CZ/Core/CZWindowState.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZEdge.h>
#include <CZ/Core/CZTime.h>

#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Louvre/Roles/LToplevelResizeSession.h>

#include <CZ/skia/core/SkRect.h>

#include <list>
#include <string>

/**
 * @brief Toplevel role for surfaces
 *
 * @anchor ltoplevelrole_detailed
 *
 * The toplevel surface role represents typical desktop windows that usually have a title and buttons to close, minimize, and maximize.
 *
 * <center><img height="250px" src="https://lh3.googleusercontent.com/PvNaxLgkjaPryJ8W_P3bDOccQu1m-zNEcI7aH_R8WggzylV5LQZtuzLTUSImThDI8IVsAI9DERF4cwvSqPAEAyjsihHuPCQlZAbvu33iMC2iXvpTswZC3RBNDyKm1YEWDnnKeCn2Qw=w2400"></center>
 *
 * The toplevel role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_toplevel) protocol.
 * 
 * @note The Wayland protocol also provides its own toplevel role but it is considered obsolete and therefore not included in Louvre.
 *
 * @section toplevel-configuration Configuration
 *
 * When a client wants to map an LToplevelRole surface, it triggers a configureRequest(), where the compositor is expected to define the size, state, bounds, and other properties
 * of the toplevel using different configure methods, such as configureSize(), configureState(), configureBounds(), etc.
 *
 * The configuration is not applied immediately, the compositor must wait for the client to ACK the changes through the atomsChanged() event which is also triggered whenever one
 * or more properties of the atoms() struct change, allowing to access previous values as well.
 *
 * @note The compositor can configure any parameters at any time, but should always wait for the atomsChanged() event to detect when they are actually applied.
 *
 * @section toplevel-state-change-requests State Change Requests
 *
 * Depending on the flags set with configureCapabilities(), clients can request to minimize or change the state() of the toplevel to @ref Maximized or @ref Fullscreen through
 * setMinimizedRequest(), set / unsetMaximizedRequest() and set / unsetFullscreenRequest().
 * The compositor is then expected to minimize or properly configure the state and size, but it's free to ignore the requests if desired.
 *
 * @note Minimized is not a toplevel @ref State and can be applied immediately.
 *
 * @section foreign-controller Foreign Toplevel Controllers and XDG Activation Tokens
 *
 * Requests, including those previously mentioned as well as `unsetMinimizedRequest()`, `activateRequest()`, and `closeRequest()`, can be triggered by external clients
 * through the [Wlr Foreign Toplevel Management](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1) and
 * the [XDG Activation](https://wayland.app/protocols/xdg-activation-v1) protocols. For a comprehensive understanding of this functionality, please refer to the
 * LForeignToplevelController and LActivationTokenManager classes documentation.
 *
 * @section toplevel-sessions Interactive Sessions
 *
 * Clients can request to start interactive move and resize sessions through startMoveRequest() and startResizeRequest(). To properly handle such requests,
 * each toplevel has an auxiliary moveSession() and resizeSession() which can handle the request for you and constrain the toplevel to the available geometry of
 * outputs (LOutput::availableGeometry()) or to custom constraints specified by you. These are simply auxiliary and you are free to not use them if desired.
 *
 * @section toplevel-decoration Decorations
 *
 * By default, all clients draw their own decorations on the toplevel surface, such as shadows, and the windowGeometry() property indicates which part of the surface
 * are decorations and which are the actual window content.
 *
 * Some clients also supportServerSideDecorations() (SSD). To enable SSD, the compositor must send a configureDecorationMode() with the @ref ServerSide value.\n
 * When the decorationMode() changes to @ref ServerSide, the client does not draw any decorations, and the windowGeometry() equals the entire surface size.\n
 * If the compositor draws additional elements that are meant to be part of the geometry (such as a title bar), the setExtraGeometry() method can be employed,
 * which allows for proper positioning of the toplevel and constraining it during interactive sessions.
 *
 * @section toplevel-position Positioning
 *
 * The toplevel position can be set with LSurface::setPos(). The default implementation of rolePos() then returns the given position minus the decoration part
 * of windowGeometry() and adds the top and left margins of extraGeometry().
 */
class CZ::LToplevelRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LToplevelRole;

    /// Decoration mode
    enum DecorationMode : UInt32
    {
        /// The client has no preferred mode
        NoPreferredMode = 0,

        /// Decorations are drawn by the client
        ClientSide = 1,

        /// Decorations are drawn by the compositor
        ServerSide = 2
    };

    /**
     * @brief Flags indicating which properties changed during a stateChanged() event.
     */
    enum Changes : UInt32
    {
        /// Indicates windowGeometry() changed.
        WindowGeometryChanged   = 1U << 0,

        /// Indicates minSize() changed.
        MinSizeChanged          = 1U << 1,

        /// Indicates maxSize() changed.
        MaxSizeChanged          = 1U << 2,

        /// Indicates state() changed.
        StateChanged            = 1U << 3,

        /// Indicates bounds() changed.
        BoundsChanged           = 1U << 4,

        /// Indicates capabilities() changed.
        CapabilitiesChanged     = 1U << 5,

        /// Indicates decorationMode() changed.
        DecorationModeChanged   = 1U << 6,

        /// Indicates serial() changed.
        SerialChanged           = 1U << 7,
    };

    /**
     * @brief Compositor capabilities flags
     *
     * Flags indicating supported compositor features, notified
     * to the client through the configureCapabilities() method.
     */
    enum Capabilities : UInt8
    {
        /// When set, the toplevel can trigger showWindowMenuRequest().
        WindowMenuCap  = static_cast<UInt8>(1) << 0,

        /// When set, the toplevel can trigger setMaximizedRequest() and unsetMaximizedRequest().
        MaximizeCap    = static_cast<UInt8>(1) << 1,

        /// When set, the toplevel can trigger setFullscreenRequest() and unsetFullscreenRequest().
        FullscreenCap  = static_cast<UInt8>(1) << 2,

        /// When set, the toplevel can trigger setMinimizedRequest().
        MinimizeCap    = static_cast<UInt8>(1) << 3
    };

    /**
     * @brief The toplevel state.
     */
    struct State
    {
        /// LToplevelRole::state()
        CZBitset<CZWindowState> windowState;

        /// LToplevelRole::windowGeometry()
        SkIRect windowGeometry {};

        /// LToplevelRole::minSize()
        SkISize minSize {};

        /// LToplevelRole::maxSize()
        SkISize maxSize {};

        /// LToplevelRole::bounds()
        SkISize bounds {};

        /// LToplevelRole::capabilities()
        CZBitset<Capabilities> capabilities { WindowMenuCap | MaximizeCap | FullscreenCap | MinimizeCap };

        /// LToplevelRole::decorationMode()
        DecorationMode decorationMode { ClientSide };

        /// LToplevelRole::serial()
        UInt32 serial { 0 };

        bool windowGeometrySet { false };
    };

    /**
     * @brief Configuration parameters sent to the client.
     *
     * @see pendingConfiguration()
     * @see findConfiguration()
     */
    struct Configuration
    {
        /// @see LToplevelRole::state() and configureState()
        CZBitset<CZWindowState> windowState;

        /// Size of the toplevel without decorations
        /// @see windowGeometry() and configureSize()
        SkISize size {};

        /// @see LToplevelRole::bounds() and configureBounds()
        SkISize bounds {};

        /// @see LToplevelRole::capabilities() and configureCapabilities()
        CZBitset<Capabilities> capabilities;

        /// @see LToplevelRole::decorationMode() and configureDecorationMode()
        DecorationMode decorationMode { ClientSide };

        /// @see LToplevelRole::serial() and pendingConfiguration()
        UInt32 serial { 0 };
    };

    /**
     * @brief Constructor of the LToplevelRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LToplevelRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LToplevelRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     *
     * @warning The `surface()` handle always remains valid during the destructor call.
     *          However, `LSurface::role()` returns `nullptr` because `LSurface::roleChanged()`
     *          is notified beforehand and requires the role to be valid.
     */
    ~LToplevelRole() noexcept;

    /**
     * @brief Find configuration by serial number.
     *
     * @warning The returned configuration must not be deleted.
     * @return A pointer to the configuration if found, otherwise `nullptr`.
     */
    const Configuration *findConfiguration(UInt32 serial) const noexcept;

    /**
     * @brief Pending configuration.
     *
     * This struct holds the last configuration parameters assigned
     * with configureState(), configureSize(), configureBounds(), etc.
     *
     * The pending configuration parameters are sent to the client once per Louvre
     * main loop iteration only if one of the configure variants was called.
     *
     * The pending serial is updated at most once per loop iteration if a configure
     * was done.
     *
     * If the pending configuration serial is equal to serial() it means the last
     * configuration was ACK by the client and there is no pending configuration.
     *
     * @see atomsChanged() to detect when the serial and other parameters change.
     */
    const Configuration &pendingConfiguration() const noexcept
    {
        return m_pendingConfiguration;
    }

    /**
     * @brief Configure the toplevel state.
     *
     * Asks the client to configure the toplevel with the given states and stores the flags in pendingConfiguration().\n
     * The state is not applied immediately, see atomsChanged() and state().
     *
     * @note A client may not support all states, in such cases unsupported states are filtered out. See supportedStates().
     *
     * @param flags The state flags to set.
     */
    void configureState(CZBitset<CZWindowState> flags) noexcept
    {
        updateSerial();
        m_flags.add(HasSizeOrStateToSend);
        m_pendingConfiguration.windowState = flags & supportedWindowStates();
    }

    /**
     * @brief Configure the toplevel size.
     *
     * Asks the client to configure the toplevel with the given size and stores it in pendingConfiguration().\n
     * The size is not applied immediately, see atomsChanged() and windowGeometry().
     *
     * @param size The size is in surface coordinates and does not include decorations.\n
     *             If either width or height is 0, the client is free to pick its own size.
     */
    void configureSize(const SkISize &size) noexcept
    {
        configureSize(size.width(), size.height());
    }

    /**
     * @brief Configure the toplevel size.
     *
     * @see configureSize()
     */
    void configureSize(Int32 width, Int32 height) noexcept
    {
        updateSerial();
        m_flags.add(HasSizeOrStateToSend);
        m_pendingConfiguration.size.fWidth = width < 0 ? 0 : width;
        m_pendingConfiguration.size.fHeight = height < 0 ? 0 : height;
    }

    /**
     * @brief Configure the toplevel decoration mode.
     *
     * Asks the client to configure the toplevel with the given decoration mode and stores it in pendingConfiguration().\n
     * The decoration mode is not applied immediately, see atomsChanged() and decorationMode().
     *
     * @note The decoration mode is @ref ClientSide by default. If a client doesn't support @ref ServerSide decorations,
     *       the configuration mode will fallback to @ref ClientSide. See supportServerSideDecorations().
     *
     * @see preferredDecorationMode()
     *
     * @param mode The decoration mode.
     */
    void configureDecorationMode(DecorationMode mode) noexcept
    {
        if ((mode != ClientSide && mode != ServerSide) || !supportsSSD())
            mode = ClientSide;

        updateSerial();
        m_flags.add(HasDecorationModeToSend);
        m_pendingConfiguration.decorationMode = mode;
    }

    /**
     * @brief Notifies the toplevel about the compositor's capabilities.
     *
     * Informs the toplevel which @ref Capabilities are supported by the compositor and stores them in pendingConfiguration().\n
     * For example, if the @ref MaximizeCap is not available, clients should hide
     * the maximize button in their decorations. Additionally, requests such as setMaximizedRequest()
     * and unsetMaximizedRequest() will not be triggered.
     *
     * The capabilities are not applied immediately, see atomsChanged() and capabilities().
     *
     * All capabilities are enabled by default.
     *
     * @note Some poorly behaved clients, such as Firefox, always expect the compositor to obey their requests
     *       and freeze when it does not. In such cases, it is better to leave @ref MaximizeCap and @ref FullscreenCap enabled.
     *
     * @param caps The supported @ref Capabilities flags.
     */
    void configureCapabilities(CZBitset<Capabilities> caps) noexcept
    {
        updateSerial();
        m_flags.add(HasCapabilitiesToSend);
        m_pendingConfiguration.capabilities = caps & (WindowMenuCap | MaximizeCap | FullscreenCap | MinimizeCap);
    }

    /**
     * @brief Asks the client to constrain its size to the specified bounds.
     *
     * Requests the client to prevent assigning a size to the toplevel surface
     * larger than the given bounds, for example, to prevent exceeding the available geometry of an output.
     *
     * The bounds are stored in pendingConfiguration() and not applied immediately, see atomsChanged() and bounds().
     *
     * @note This is merely a suggestion, clients are free to ignore it.
     *
     * @param bounds The suggested maximum size. Setting a component to 0 disables the constraint for that component.
     */
    void configureBounds(const SkISize &bounds) noexcept
    {
        configureBounds(bounds.width(), bounds.height());
    }

    /**
     * @brief Asks the client to constrain its size to the specified bounds.
     *
     * @see configureBounds()
     */
    void configureBounds(Int32 width, Int32 height) noexcept
    {
        updateSerial();
        m_flags.add(HasBoundsToSend);
        m_pendingConfiguration.bounds.fWidth = width < 0 ? 0 : width;
        m_pendingConfiguration.bounds.fHeight = height < 0 ? 0 : height;
    }

    /**
     * @brief Current atomic properties.
     *
     * This struct contains all the current toplevel atomic properties, which are updated
     * each time atomsChanged() is triggered.
     *
     * The current properties can also be accessed via aliases such as windowGeometry(), state(), bounds(), etc.
     */
    const State &state() const noexcept { return m_current; }

    /**
     * @brief Current toplevel state.
     *
     * Flags representing the current toplevel @ref State.
     *
     * The state flags can also be checked via aliases such as activated(), maximized(), fullscreen(), etc.
     *
     * @see configureState().
     *
     * @note This is an alias for Atoms::state.
     */
    CZBitset<CZWindowState> windowState() const noexcept { return state().windowState; }

    /**
     * @brief Returns the states supported by the toplevel.
     *
     * - @ref Activated, @ref Maximized , @ref Fullscreen , and @ref Resizing are always supported.
     * - If any of the tiled states is supported, all are supported.
     * - The @ref Suspended state support is independent of other states.
     *
     * @return A bitset representing the supported states.
     */
    CZBitset<CZWindowState> supportedWindowStates() const noexcept { return m_supportedWindowStates; }

    /**
     * @brief Checks if the toplevel state() includes the @ref Activated flag.
     *
     * @return `true` if the flag is present, `false` otherwise.
     */
    bool isActivated() const noexcept { return windowState().has(CZWinActivated); }

    /**
     * @brief Checks if the toplevel state() includes the @ref Maximized flag.
     *
     * @return `true` if the flag is present, `false` otherwise.
     */
    bool isMaximized() const noexcept { return windowState().has(CZWinMaximized); }

    /**
     * @brief Checks if the toplevel state() includes the @ref Fullscreen flag.
     *
     * @return `true` if the flag is present, `false` otherwise.
     */
    bool isFullscreen() const noexcept { return windowState().has(CZWinFullscreen); }

    /**
     * @brief Checks if the toplevel state() includes the @ref Resizing flag.
     *
     * @return `true` if the flag is present, `false` otherwise.
     */
    bool isResizing() const noexcept { return windowState().has(CZWinResizing); }

    /**
     * @brief Checks if the toplevel state() includes the @ref Suspended flag.
     *
     * @return `true` if the flag is present, `false` otherwise.
     */
    bool isSuspended() const noexcept { return windowState().has(CZWinSuspended); }

    bool isMinimized() const noexcept { return m_flags.has(IsMinimized); }

    void setMinimized(bool minimized) noexcept;

    /**
     * @brief Window geometry in surface coordinates
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a rect within the toplevel's surface that excludes its decorations (typically shadows).
     *
     * @note This is an alias for Atoms::windowGeometry.
     */
    SkIRect windowGeometry() const noexcept { return state().windowGeometry; }

    /**
     * @brief Gets the minimum size of the toplevel in surface coordinates.
     *
     * Components with a value of 0 indicate the limit is disabled.
     */
    SkISize minSize() const noexcept { return state().minSize; }

    /**
     * @brief Gets the maximum size of the toplevel in surface coordinates.
     *
     * Components with a value of 0 indicate the limit is disabled.
     */
    const SkISize maxSize() const noexcept { return state().maxSize; }

    /**
     * @brief Check if the provided size falls within the range defined by minSize() and maxSize().
     *
     * @param size The size to be checked.
     * @return `true` if the size is within the specified range, `false` otherwise.
    */
    bool sizeInRange(const SkISize &size) const noexcept
    {
        return (minSize().width() == 0 || minSize().width() <= size.width()) &&
               (maxSize().width() == 0 || maxSize().width() >= size.width()) &&
               (minSize().height() == 0 || minSize().height() <= size.height()) &&
               (maxSize().height() == 0 || maxSize().height() >= size.height());
    }

    /**
     * @brief Constraints during move/resize sessions.
     *
     * Returns the left, top, right, and bottom constraints in global-compositor coordinates
     * during a moveSession() and resizeSession() so that the toplevel stays within the given
     * LOutput::availableGeometry(). See LToplevelMoveSession::setConstraints() and
     * LToplevelResizeSession::setConstraints()
     *
     * @param includeExtraGeometry If `true`, extraGeometry() is considered.
     */
    LMargins calculateConstraintsFromOutput(LOutput *output) const noexcept;

    /**
     * @brief Current bounds.
     *
     * Suggested size constraints, notified to the client via configureBounds().
     *
     * @note This is an alias for Atoms::bounds.
     */
    SkISize bounds() const noexcept { return state().bounds; }

    /**
     * @brief Current capabilities.
     *
     * Flags representing the @ref Capabilities supported by the compositor, as notified to the client via configureCapabilities().
     *
     * @note This is an alias for Atoms::capabilities.
     */
    CZBitset<Capabilities> capabilities() const noexcept { return state().capabilities; }

    /**
     * @brief The current decoration mode.
     *
     * @see configureDecorationMode()
     * @see atomsChanged()
     *
     * @note This is an alias for Atoms::decorationMode.
     */
    DecorationMode decorationMode() const noexcept { return state().decorationMode; }

    /**
     * @brief Gets the preferred decoration mode set by the client.
     *
     * @return The preferred decoration mode value.
     */
    DecorationMode preferredDecorationMode() const noexcept { return m_preferredDecorationMode; }

    /**
     * @brief Check if the toplevel supports server-side decorations.
     */
    bool supportsSSD() const noexcept { return m_xdgDecorationRes.get() != nullptr; }

    /**
     * @brief Last configuration serial ACK by the client.
     *
     * @see pendingConfiguration().
     *
     * @note This is an alias for Atoms::serial.
     */
    UInt32 serial() const noexcept { return state().serial; }

    /**
     * @brief Sets the exclusive output hint.
     *
     * This is an optional hint that can be used to keep a reference of
     * in which `LOutput` a toplevel is positioned while maximized or in fullscreen mode.
     *
     * The default implementation of `LOutput::paintGL()` uses this information to prevent displaying
     * the given toplevel on other outputs.
     *
     * @see exclusiveOutput()
     *
     * @param output The given output or `nullptr` to unset.
     */
    void setExclusiveOutput(LOutput *output) noexcept;

    /**
     * @brief Exclusive output hint.
     *
     * Returns the exclusive output hint set with setExclusiveOutput(), or `nullptr` if unset.
     */
    LOutput *exclusiveOutput() const override { return m_exclusiveOutput; }

    /**
     * @brief Utility for handling interactive moving sessions.
     *
     * @see startMoveRequest()
     * @see LSeat::toplevelMoveSessions()
     */
    LToplevelMoveSession &moveSession() const noexcept { return m_moveSession; }

    /**
     * @brief Utility for handling interactive resizing sessions.
     *
     * @see startResizeRequest()
     * @see LSeat::toplevelResizeSessions()
     */
    LToplevelResizeSession &resizeSession() const noexcept { return m_resizeSession; }

    /**
     *  @brief [xdg_toplevel](https://wayland.app/protocols/xdg-shell#xdg_toplevel) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgToplevel *xdgToplevelResource() const;

    /**
     *  @brief [xdg_surface](https://wayland.app/protocols/xdg-shell#xdg_surface) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgSurface *xdgSurfaceResource() const;

    /**
     * @brief Gets the application ID associated with the toplevel window.
     *
     * @see appIdChanged()
     *
     * @return A string containing the application ID (e.g., "org.cuarzosoftware.Desk").
     */
    const std::string &appId() const noexcept { return m_appId; }

    /**
     * @brief Gets the window title of the toplevel.
     *
     * @see titleChanged()
     *
     * @return A string representing the window title.
     */
    const std::string &title() const noexcept { return m_title; }

    const std::list<LToplevelRole*> &childToplevels() const noexcept { return m_childToplevels; }
    const std::list<LPopupRole*> &childPopups() const noexcept { return m_childPopups; }

    /**
     * @brief Closes the toplevel.
     *
     * Requests the client to close the toplevel (equivalent to pressing the close button on the window).
     *
     * @note The client may choose to ignore this request, or show a dialog to ask the user to save their data, etc.
     */
    void close() noexcept;

    /**
     * @brief Vector of foreign controllers
     *
     * @see The LForeignToplevelController class for more details.
     */
    const std::vector<LForeignToplevelController*> &foreignControllers() const noexcept { return m_foreignControllers; }

    /**
     * @brief LForeignToplevelController triggering a request.
     *
     * If this method does not return `nullptr` during a state-changing request, it means
     * it is being triggered by the specified LForeignToplevelController::client().
     *
     * @see LForeignToplevelController class for more details.
     */
    LForeignToplevelController *requesterController() const noexcept { return m_requesterController; }

    /**
     * @brief Vector of foreign handles.
     *
     * A vector containing all the handles used by foreign clients to identify this toplevel.
     *
     * @see foreignHandleFilter() class for more details.
     */
    const std::vector<Protocols::ForeignToplevelList::RForeignToplevelHandle*> foreignHandles() const noexcept { return m_foreignToplevelHandles; }

    /**
     * @brief ID used by foreign clients to identify the toplevel
     *
     * This ID is used by foreign clients using the [Foreign Toplevel List](https://wayland.app/protocols/ext-foreign-toplevel-list-v1#ext_foreign_toplevel_handle_v1:event:identifier) protocol (see `foreignHandleFilter()` for details).
     * 
     * The string is initially empty and is updated after the toplevel is mapped or re-mapped.
     */
    const std::string &foreignToplevelListIdentifier() const noexcept { return m_identifier; }

    /**
     * @brief Auxiliary previous rect.
     *
     * This auxiliary rect is used by the default implementation
     * to save the position and size of the toplevel window before it is maximized
     * or switched to fullscreen, allowing it to be restored later.
     */
    SkIRect prevRect { 0, 0, 0, 0 };

    ///@}

/// @name Virtual Methods
/// @{
    /**
     * @brief Position of the surface according to the role.
     *
     * Override this virtual method if you need to define your own logic for positioning the toplevel.
     *
     * The default implementation returns the position set with LSurface::setPos() minus the decoration part
     * of windowGeometry() and adds the top and left margins of extraGeometry().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp rolePos
     */
    virtual SkIPoint rolePos() const override;

    /**
     * @brief Configuration request.
     *
     * This request is triggered each time the client intends to map the toplevel surface.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

    /**
     * @brief Notifies a change in atomic properties.
     *
     * This event is triggered each time one or more of the atoms() change.
     *
     * @param changes Flags indicating which properties in atoms() have changed.
     * @param prevAtoms Structure containing the previous values of atoms().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp atomsChanged
     */
    virtual void stateChanged(CZBitset<Changes> changes, const State &prev);

    /**
     * @brief Client request to initiate an interactive move session.
     *
     * The default implementation utilizes the moveSession() utility to handle
     * the session, and ignores it if the toplevel is in @ref Fullscreen mode.
     *
     * The triggering event helps differentiate whether it is a pointer, touch,
     * or other type of session.
     *
     * @param triggeringEvent The event that triggered the move session.
     *
     * @see moveSession()
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startMoveRequest
     */
    virtual void startMoveRequest(const CZEvent &triggeringEvent);

    /**
     * @brief Client request to initiate an interactive resize session.
     *
     * The default implementation utilizes the resizeSession() utility to handle
     * the session, and ignores it if the toplevel is in @ref Fullscreen mode.
     *
     * The triggering event helps differentiate whether it is a pointer, touch,
     * or other type of session.
     *
     * @param triggeringEvent The event that triggered the resize session.
     * @param edge The edge or corner being dragged.
     *
     * @see resizeSession()
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startResizeRequest
     */
    virtual void startResizeRequest(const CZEvent &triggeringEvent, CZBitset<CZEdge> edge);

    /**
     * @brief Request to activate.
     *
     * Triggered by a an LActivationTokenManager::token() or requesterController() expecting the compositor to configure the toplevel with the @ref Activated state.
     *
     * @see configureState(), requesterController() and LForeignToplevelController.
     *
     * @note The compositor is free to ignore this request.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp activateRequest
     */
    virtual void activateRequest();

    /**
     * @brief Request to maximize.
     *
     * Triggered by the client or a requesterController() expecting the compositor to configure the toplevel with the @ref Maximized state. See configureState().
     *
     * @note This event is only triggered if the @ref MaximizeCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp setMaximizedRequest
     */
    virtual void setMaximizedRequest();

    /**
     * @brief Request to unmaximize.
     *
     * Triggered by the client or a requesterController() expecting the compositor to configure the toplevel without the @ref Maximized state. See configureState().
     *
     * @note This event is only triggered if the @ref MaximizeCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetMaximizedRequest
     */
    virtual void unsetMaximizedRequest();

    /**
     * @brief Request to set fullscreen mode.
     *
     * Triggered by the client or a requesterController() expecting the compositor to configure the toplevel with the @ref Fullscreen state. See configureState().
     *
     * @note This event is only triggered if the @ref FullscreenCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * @param destOutput Output on which the client wishes to display the toplevel. If `nullptr` the compositor must choose the output.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setFullscreenRequest
     */
    virtual void setFullscreenRequest(LOutput *destOutput);

    /**
     * @brief Request to unset fullscreen mode.
     *
     * Triggered by the client or a requesterController() expecting the compositor to configure the toplevel without the @ref Fullscreen state. See configureState().
     *
     * @note This event is only triggered if the @ref FullscreenCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetFullscreenRequest
     */
    virtual void unsetFullscreenRequest();

    /**
     * @brief Minimize request.
     *
     * Triggered by the client or a requesterController() expecting the compositor to minimize the toplevel window.
     *
     * @see LSurface::setMinimized(), requesterController() and LForeignToplevelController.
     *
     * @note This event is only triggered if the @ref MinimizeCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setMinimizedRequest
     */
    virtual void setMinimizedRequest();

    /**
     * @brief Unminimize request.
     *
     * Triggered by a requesterController() expecting the compositor to unminimize the toplevel window.
     *
     * @see LSurface::setMinimized(), requesterController() and LForeignToplevelController.
     *
     * @note This event is only triggered if the @ref MinimizeCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetMinimizedRequest
     */
    virtual void unsetMinimizedRequest();

    virtual void minimizedChanged();

    /**
     * @brief Close request.
     *
     * Triggered by a requesterController() expecting the compositor to close() the toplevel window.
     *
     * @see close(), requesterController() and LForeignToplevelController.
     *
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp closeRequest
     */
    virtual void closeRequest();

    /**
     * @brief Foreign toplevel controller filter
     *
     * This method allows you to filter which foreign clients can control this toplevel. See LForeignToplevelController for more details.
     *
     * If accepted, a new LForeignToplevelController object will be added to the foreignControllers() vector.
     *
     * @param manager The `GForeignToplevelManager` resource requesting to control the toplevel.
     *
     * @return `true` to allow the foreign client to control the toplevel, `false` to deny it.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp foreignControllerFilter
     */
    virtual bool foreignControllerFilter(Protocols::ForeignToplevelManagement::GForeignToplevelManager *manager);

    /**
     * @brief Filter for foreign toplevel handle requests
     *
     * This method allows you to control which foreign clients can obtain a handle for this toplevel.
     *
     * The handle enables clients to identify other clients' toplevels and, in conjunction with other protocols,
     * perform operations such as selecting which toplevel to capture using protocols like Image Copy Capture.
     *
     * If accepted, a new `ForeignToplevelList::RForeignToplevelHandle` object will be added to the foreignHandles() vector.\n
     * Alternatively, you can disable the protocol for specific clients using LCompositor::globalsFilter().
     *
     * @note This interface is related to the [Foreign Toplevel List](https://wayland.app/protocols/ext-foreign-toplevel-list-v1) protocol. Do not confuse it with the [Wlr Foreign Toplevel Management](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1) protocol (see foreignControllerFilter()).
     *
     * This protocol can be tested with clients like [lswt](https://git.sr.ht/~leon_plickat/lswt).
     *
     * @param foreignList The `GForeignToplevelList` resource requesting to get a handle.
     *
     * @return `true` to allow the foreign client to obtain a handle for this toplevel, `false` to deny it.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp foreignHandleFilter
     */
    virtual bool foreignHandleFilter(Protocols::ForeignToplevelList::GForeignToplevelList *foreignList);

    /**
     * @brief Show window menu request.
     *
     * Triggered by the client expecting the compositor to display a popup menu with options
     * for minimizing, maximizing and/or turning the toplevel into fullscreen mode.
     *
     * @note This event is only triggered if the @ref WindowMenuCap is set. See configureCapabilities().
     * @note The compositor is free to ignore this request.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp showWindowMenuRequest
     */
    virtual void showWindowMenuRequest(const CZEvent &triggeringEvent, Int32 x, Int32 y);

    /**
     * @brief Notifies a change in the title string.
     *
     * @see title()
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp titleChanged
     */
    virtual void titleChanged();

    /**
     * @brief Notifies a change in the App ID string.
     *
     * @see appId()
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp appIdChanged
     */
    virtual void appIdChanged();

    /**
     * @brief Notifies the client has changed its preferred decoration mode.
     *
     * @see preferredDecorationMode()
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp preferredDecorationModeChanged
     */
    virtual void preferredDecorationModeChanged();

/// @}

private:
    friend class LCompositor;
    friend class LPopupRole;
    friend class LToplevelResizeSession;
    friend class LForeignToplevelController;
    friend class Protocols::XdgShell::RXdgSurface;
    friend class Protocols::XdgShell::RXdgToplevel;
    friend class Protocols::XdgDecoration::RXdgToplevelDecoration;
    friend class Protocols::ForeignToplevelManagement::RForeignToplevelHandle;
    friend class Protocols::ForeignToplevelManagement::GForeignToplevelManager;
    friend class Protocols::ForeignToplevelList::RForeignToplevelHandle;

    enum Flags : UInt32
    {
        IsMinimized                 = 1U << 0,
        ForceRemoveActivatedFlag    = 1U << 1,
        HasPendingInitialConf       = 1U << 2,
        HasSizeOrStateToSend        = 1U << 3,
        HasDecorationModeToSend     = 1U << 4,
        HasBoundsToSend             = 1U << 5,
        HasCapabilitiesToSend       = 1U << 6,
        HasPendingFirstMap          = 1U << 7
    };

    void cacheCommit() noexcept override;
    void applyCommit() noexcept override;
    void applyState(State *pending) noexcept;

    void handleParentChange() noexcept;
    void sendPendingConfiguration() noexcept;
    void updateSerial() noexcept;
    void reset(State *pending) noexcept;

    void setTitle(const char *title) noexcept;
    void setAppId(const char *appId) noexcept;
    void setParent(LToplevelRole *parent) noexcept;

    State m_current {};
    State m_pending {};
    std::list<std::shared_ptr<State>> m_cache;

    CZBitset<Flags> m_flags { HasPendingInitialConf | HasPendingFirstMap };

    std::string m_appId;
    std::string m_title;

    std::list<LPopupRole*> m_childPopups;
    std::list<LToplevelRole*> m_childToplevels;
    std::list<LToplevelRole*>::iterator m_parentLink;

    // Guaranteed to be supported by xdg_toplevel v1
    CZBitset<CZWindowState> m_supportedWindowStates { CZWinActivated | CZWinMaximized | CZWinFullscreen | CZWinResizing };

    CZWeak<Protocols::XdgDecoration::RXdgToplevelDecoration> m_xdgDecorationRes;
    DecorationMode m_preferredDecorationMode { NoPreferredMode };

    mutable LToplevelResizeSession m_resizeSession { this };
    mutable LToplevelMoveSession m_moveSession { this };

    CZWeak<LOutput> m_exclusiveOutput;

    Configuration m_pendingConfiguration, m_lastACKConfiguration;
    std::list<Configuration> m_sentConfigurations;

    // This is for clients that do maximize or fullscreen requests before the first conf
    CZWindowState m_requestedStateBeforeConf {};
    CZWeak<LOutput> m_fullscreenOutputBeforeConf;

    /* Foreign toplevel management */
    std::vector<LForeignToplevelController*> m_foreignControllers;
    LForeignToplevelController *m_requesterController { nullptr }; // Only set during requests

    /* Foreign toplevel list */
    std::string m_identifier;
    std::vector<Protocols::ForeignToplevelList::RForeignToplevelHandle*> m_foreignToplevelHandles;
};

#endif // LTOPLEVELROLE_H
