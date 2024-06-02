#ifndef LTOPLEVELROLE_H
#define LTOPLEVELROLE_H

#include <LMargins.h>
#include <LBaseSurfaceRole.h>
#include <LBitset.h>
#include <LSize.h>
#include <LRect.h>
#include <string>
#include <LEdge.h>
#include <LTime.h>

#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>

#include <list>

/**
 * @brief Toplevel role for surfaces
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
 * or more properties of the atoms() struct change, allowing you to also access previous values as well.
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
 * Some clients also support server-side decorations (SSD). To enable SSD, the compositor must send a configureDecorationMode() with the @ref ServerSide value.\n
 * When the decorationMode() changes to @ref ServerSide, the client does not draw any decorations, and the windowGeometry() equals the entire surface size.\n
 * If the compositor draws additional elements that are meant to be part of the geometry (such as a title bar), the setExtraGeometry() method can be employed,
 * which allows for proper positioning of the toplevel and constraining it during interactive sessions.
 *
 * @section toplevel-position Positioning
 *
 * The toplevel position can be set with LSurface::setPos(). The default implementation of rolePos() then returns the given position minus the decoration part
 * of windowGeometry() and adds the top and left margins of extraGeometry().
 */
class Louvre::LToplevelRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LToplevelRole;

    /**
     * @brief Flags indicating the possible states of a Toplevel.
     */
    enum State : UInt16
    {
        /// No state
        NoState     = static_cast<UInt16>(0),

        /// Maximized
        Maximized   = static_cast<UInt16>(1),

        /// Fullscreen mode
        Fullscreen  = static_cast<UInt16>(1) << 1,

        /// In interactive resizing
        Resizing    = static_cast<UInt16>(1) << 2,

        /// Activated (its decorations stand out from others)
        Activated   = static_cast<UInt16>(1) << 3,

        /// Tiled left (since 2)
        TiledLeft   = static_cast<UInt16>(1) << 4,

        /// Tiled right (since 2)
        TiledRight  = static_cast<UInt16>(1) << 5,

        /// Tiled top (since 2)
        TiledTop    = static_cast<UInt16>(1) << 6,

        /// Tiled bottom (since 2)
        TiledBottom = static_cast<UInt16>(1) << 7,

        /// Suspended (since 6)
        Suspended   = static_cast<UInt16>(1) << 8,
    };

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
     * @brief Flags indicating which atomic properties have changed during an atomsChanged() event.
     *
     * @see @ref Atoms
     * @see atoms()
     * @see atomsChanged()
     */
    enum AtomChanges : UInt32
    {
        /// Indicates windowGeometry() changed.
        WindowGeometryChanged   = static_cast<UInt32>(1) << 0,

        /// Indicates minSize() changed.
        MinSizeChanged          = static_cast<UInt32>(1) << 1,

        /// Indicates maxSize() changed.
        MaxSizeChanged          = static_cast<UInt32>(1) << 2,

        /// Indicates state() changed.
        StateChanged            = static_cast<UInt32>(1) << 3,

        /// Indicates bounds() changed.
        BoundsChanged           = static_cast<UInt32>(1) << 4,

        /// Indicates capabilities() changed.
        CapabilitiesChanged     = static_cast<UInt32>(1) << 5,

        /// Indicates decorationMode() changed.
        DecorationModeChanged   = static_cast<UInt32>(1) << 6,

        /// Indicates serial() changed.
        SerialChanged           = static_cast<UInt32>(1) << 7,
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
     * @brief Atomic properties
     *
     * This struct contains all `LToplevelRole` properties that should be handled
     * simultaneously during an `atomsChanged()` event.
     *
     * @see atoms()
     */
    struct Atoms
    {
        /// LToplevelRole::state()
        LBitset<State> state;

        /// LToplevelRole::windowGeometry()
        LRect windowGeometry;

        /// LToplevelRole::minSize()
        LSize minSize;

        /// LToplevelRole::maxSize()
        LSize maxSize;

        /// LToplevelRole::bounds()
        LSize bounds;

        /// LToplevelRole::capabilities()
        LBitset<Capabilities> capabilities { WindowMenuCap | MaximizeCap | FullscreenCap | MinimizeCap };

        /// LToplevelRole::decorationMode()
        DecorationMode decorationMode { ClientSide };

        /// LToplevelRole::serial()
        UInt32 serial { 0 };
    };

    /**
     * @brief Configuration parameters sent to the client.
     *
     * @see configureState()
     * @see configureSize()
     * @see configureBounds()
     * @see configureCapabilities()
     * @see configureDecorationMode()
     * @see pendingConfiguration()
     */
    struct Configuration
    {
        /// @see LToplevelRole::state()
        LBitset<State> state;

        /// Size of the toplevel without decorations
        LSize size;

        /// @see LToplevelRole::bounds()
        LSize bounds;

        /// @see LToplevelRole::capabilities()
        LBitset<Capabilities> capabilities;

        /// @see LToplevelRole::decorationMode()
        DecorationMode decorationMode { ClientSide };

        /// @see LToplevelRole::serial()
        UInt32 serial { 0 };
    };

    /**
     * @brief Sets the exclusive output hint
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
     * @brief Exclusive output hint
     *
     * Returns the exclusive output hint set with `setExclusiveOutput()`, or `nullptr` if unset.
     */
    virtual LOutput *exclusiveOutput() const override
    {
        return m_exclusiveOutput;
    }

    /**
     * @brief Sets extra geometry margins.
     *
     * This optional method allows you to modify the `extraGeometry()` property, providing additional margins
     * for each toplevel edge to be considered part of the geometry. For example, when using
     * server-side decorations, the compositor could draw a custom title bar or borders that
     * are not part of the client surfaces. This method allows you to define those sizes and avoids the need
     * to override `rolePos()`, the move and resizing constraints, and other default implementations such as
     * LOutput::geometryChanged().
     *
     * @note This does not affect the `windowGeometry()` property and the functioning of `configureSize()`.
     *
     * @see extraGeometry()
     *
     * @param margins Left, top, right, bottom margins.
     */
    void setExtraGeometry(const LMargins &margins) noexcept
    {
        m_extraGeometry = margins;
    }

    /**
     * @brief Extra geometry hint.
     *
     * Set to (0,0,0,0) by default.
     *
     * @see setExtraGeometry()
     */
    const LMargins &extraGeometry() const noexcept
    {
        return m_extraGeometry;
    }

    LRect prevRect;

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
     */
    ~LToplevelRole() = default;

    LCLASS_NO_COPY(LToplevelRole)

    /**
     * @brief Find configuration by serial number.
     *
     * @warning The returned configuration must not be deleted.
     * @return A pointer to the configuration if found, otherwise `nullptr`.
     */
    const Configuration *findConfiguration(UInt32 serial) const noexcept;

    /**
     * @brief Returns the states supported by the toplevel.
     *
     * - @ref Activated, @ref Maximized , @ref Fullscreen , and @ref Resizing are always supported.
     * - If any of the tiled states is supported, all are supported.
     * - The @ref Suspended state support is independent of other states.
     *
     * @return A bitset representing the supported states.
     */
    LBitset<State> supportedStates() const noexcept
    {
        return m_supportedStates;
    }

    /**
     * @brief Pending configuration.
     *
     * This struct holds the last configuration parameters assigned
     * with `configureState()`, `configureSize()`, `configureBounds()`, etc.
     *
     * The pending configuration parameters are sent to the client once per Louvre
     * main loop iteration only if one of the configure variants was called.
     *
     * The pending serial is updated at most once per loop iteration if a configure
     * was done.
     *
     * If the pending configuration serial is equal to `serial()` it means the last
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
    void configureState(LBitset<State> flags) noexcept
    {
        updateSerial();
        m_flags.add(HasSizeOrStateToSend);
        m_pendingConfiguration.state = flags & supportedStates();
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
    void configureSize(const LSize &size) noexcept
    {
        configureSize(size.w(), size.h());
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
        m_pendingConfiguration.size.setW(width < 0 ? 0 : width);
        m_pendingConfiguration.size.setH(height < 0 ? 0 : height);
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
        if ((mode != ClientSide && mode != ServerSide) || !supportServerSideDecorations())
            mode = ClientSide;

        updateSerial();
        m_flags.add(HasDecorationModeToSend);
        m_pendingConfiguration.decorationMode = mode;
    }

    void configureCapabilities(LBitset<Capabilities> caps) noexcept
    {
        updateSerial();
        m_flags.add(HasCapabilitiesToSend);
        m_pendingConfiguration.capabilities = caps & (WindowMenuCap | MaximizeCap | FullscreenCap | MinimizeCap);
    }

    void configureBounds(const LSize &bounds) noexcept
    {
        configureBounds(bounds.w(), bounds.h());
    }

    void configureBounds(Int32 width, Int32 height) noexcept
    {
        updateSerial();
        m_flags.add(HasBoundsToSend);
        m_pendingConfiguration.bounds.setW(width < 0 ? 0 : width);
        m_pendingConfiguration.bounds.setH(height < 0 ? 0 : height);
    }

    const Atoms &atoms() const noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    LBitset<Capabilities> capabilities() const noexcept
    {
        return atoms().capabilities;
    }

    /**
     * @brief Window geometry in surface coordinates
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a rect within the toplevel's surface that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const noexcept
    {
        return atoms().windowGeometry;
    }

    LBitset<State> state() const noexcept
    {
        return atoms().state;
    }

    DecorationMode decorationMode() const noexcept
    {
        return atoms().decorationMode;
    }

    bool activated() const noexcept
    {
        return state().check(Activated);
    }

    bool maximized() const noexcept
    {
        return state().check(Maximized);
    }

    bool fullscreen() const noexcept
    {
        return state().check(Fullscreen);
    }

    bool tiled() const noexcept
    {
        return state().check(TiledLeft | TiledTop | TiledRight | TiledBottom);
    }

    bool resizing() const noexcept
    {
        return state().check(Resizing);
    }

    bool suspended() const noexcept
    {
        return state().check(Suspended);
    }

    UInt32 serial() const noexcept
    {
        return atoms().serial;
    }

    /**
     * @brief Closes the Toplevel.
     *
     * Requests to close the toplevel (equivalent to pressing the close button on the window).
     */
    void close() noexcept;

    /**
     * @brief Get the application ID associated with the toplevel window.
     *
     * @return A string containing the application ID (e.g., "org.cuarzosoftware.Desk").
     */
    const std::string &appId() const noexcept
    {
        return m_appId;
    }

    /**
     * @brief Get the window title of the toplevel.
     *
     * @return A string representing the window title.
     */
    const std::string &title() const noexcept
    {
        return m_title;
    }

    /**
     * @brief Get the minimum size of the toplevel in surface coordinates.
     *
     * If one of the axis is 0, it means that axis has no expected minimum size.
     *
     * @return The minimum size as a Louvre::LSize object.
     */
    const LSize &minSize() const noexcept
    {
        return atoms().minSize;
    }

    /**
     * @brief Get the maximum size of the toplevel in surface coordinates.
     *
     * If one of the axis is 0, it means that axis has no expected maximum size.
     *
     * @return The maximum size as a Louvre::LSize object.
     */
    const LSize &maxSize() const noexcept
    {
        return atoms().maxSize;
    }

    /**
     * @brief Check if the provided size falls within the range defined by minSize() and maxSize().
     *
     * @param size The size to be checked.
     * @return `true` if the size is within the specified range, `false` otherwise.
    */
    bool sizeInRange(const LSize &size) const noexcept
    {
        return (minSize().w() <= size.w() || minSize().w() == 0) &&
               (maxSize().w() >= size.w() || maxSize().w() == 0) &&
               (minSize().h() <= size.h() || minSize().h() == 0) &&
               (maxSize().h() >= size.h() || maxSize().h() == 0);
    }

    /**
     * @brief Get the preferred decoration mode set by the client.
     *
     * @return The preferred decoration mode value.
     */
    DecorationMode preferredDecorationMode() const noexcept
    {
        return m_preferredDecorationMode;
    }

    bool supportServerSideDecorations() const noexcept
    {
        return m_xdgDecorationRes.get() != nullptr;
    }

    /**
     *  @brief [xdg_toplevel](https://wayland.app/protocols/xdg-shell#xdg_toplevel) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgToplevel *xdgToplevelResource() const;

    /**
     *  @brief [xdg_surface](https://wayland.app/protocols/xdg-shell#xdg_surface) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgSurface *xdgSurfaceResource() const;

    /**
     * @name Interactive toplevel Movement
     *
     * These utility methods simplify the management of interactive toplevel moving sessions.
     *
     * @note Using these methods is optional.
     *
     * @see LToplevelRole::startMoveRequest()
     */

    ///@{

    // TODO
    LToplevelMoveSession &moveSession() const noexcept
    {
        return m_moveSession;
    }

    ///@}

    /**
     * @name Interactive Toplevel Resizing
     *
     * These utility methods simplify the management of interactive toplevel resizing sessions.
     *
     * @note Using these methods is optional.
     *
     * @see LToplevelRole::startResizeRequest()
     * @see LToplevelRole::geometryChanged()
     */

    ///@{

    // TODO
    LToplevelResizeSession &resizeSession() const noexcept
    {
        return m_resizeSession;
    }

    ///@}

/// @name Virtual Methods
/// @{
    /**
     * @brief Position of the surface according to the role.
     *
     * Override this virtual method if you wish to define your own logic for positioning the Toplevel.
     *
     * The default implementation of rolePos() returns the position assigned by the compositor
     * with LSurface::setPos() minus the (x, y) coords of its window geometry.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Request to start an interactive move session
     *
     * Override this virtual method if you wish to be notified when the client wishes to start an interactive move session.
     *
     * @see LPointer::startMovingToplevel()
     *
     * TODO
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startMoveRequest
     */
    virtual void startMoveRequest(const LEvent &triggeringEvent);

    /**
     * @brief Request to start an interactive resize session
     *
     * Override this virtual method if you want to be notified when the client wants to start an interactive resize session.
     *
     * @see LPointer::startResizingToplevel()
     *
     * @param edge Which edge or corner is being dragged.
     *
     * TODO add params
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startResizeRequest
     */
    virtual void startResizeRequest(const LEvent &triggeringEvent, LBitset<LEdge> edge);

    /**
     * @brief Request for configuration
     *
     * Override this virtual method if you want to be notified when the client wants the compositor to configure the Toplevel.\n
     * This request occurs when the toplevel is created and each time it gets remapped after being previously unmapped.
     *
     * @note If you do not explicitly configure the toplevel, Louvre will internally invoke `configure(pendingState())`.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

    virtual void atomsChanged(LBitset<AtomChanges> changes, const Atoms &prevAtoms);

    LMargins calculateConstraintsFromOutput(LOutput *output, bool includeExtraGeometry = true) const noexcept;

    /**
     * @brief Request to maximize
     *
     * Override this virtual method if you wish to be notified when the client intends to maximize the toplevel.\n
     * It is recommended to respond to this request with a configure() event, preferably with the Louvre::LToplevelRole::Maximized flag.
     * If you have no intention of maximizing it, you may choose to ignore the request or configure it according to your preferences.
     *
     * @note If you do not explicitly configure the toplevel, Louvre will internally invoke `configure(pendingState())` after this request.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp setMaximizedRequest
     */
    virtual void setMaximizedRequest();

    /**
     * @brief Request to unmaximize
     *
     * Override this virtual method if you wish to be notified when the client intends to unmaximize the toplevel.\n
     * It is recommended to respond to this request with a configure() event, preferably without the Louvre::LToplevelRole::Maximized flag.
     * If you have no intention of unmaximizing it, you may choose to ignore the request or configure it according to your preferences.
     *
     * @note If you do not explicitly configure the toplevel, Louvre will internally invoke `configure(pendingState())` after this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetMaximizedRequest
     */
    virtual void unsetMaximizedRequest();

    /**
     * @brief Request to set fullscreen mode
     *
     * Override this virtual method if you wish to be notified when the client intends to set the toplevel into fullscreen mode.\n
     * It is recommended to respond to this request with a configure() event, preferably with the Louvre::LToplevelRole::Fullscreen flag.
     * If you have no intention of setting it fullscreen, you may choose to ignore the request or configure it according to your preferences.
     *
     * @note If you do not explicitly configure the toplevel, Louvre will internally invoke `configure(pendingState())` after this request.
     *
     * @param destOutput Output on which the client wishes to display the toplevel. If it is `nullptr` the compositor must choose the output.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setFullscreenRequest
     */
    virtual void setFullscreenRequest(LOutput *destOutput);

    /**
     * @brief Request to unset fullscreen mode
     *
     * Override this virtual method if you wish to be notified when the client intends to unset the toplevel fullscreen mode.\n
     * It is recommended to respond to this request with a configure() event, preferably without the Louvre::LToplevelRole::Fullscreen flag.
     * If you have no intention of unsetting the fullscreen mode, you may choose to ignore the request or configure it according to your preferences.
     *
     * @note If you do not explicitly configure the toplevel, Louvre will internally invoke `configure(pendingState())` after this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetFullscreenRequest
     */
    virtual void unsetFullscreenRequest();

    /**
     * @brief Minimize request.
     *
     * Request the compositor to minimize the toplevel window.
     *
     * @note The compositor is free to ignore this request.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setMinimizedRequest
     */
    virtual void setMinimizedRequest();

    /**
     * @brief Show window menu request.
     *
     * Request the compositor to display a popup menu with options
     * for minimizing, maximizing or turning the toplevel into fullscreen mode.
     *
     * @note The compositor is free to ignore this request.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp showWindowMenuRequest
     */
    virtual void showWindowMenuRequest(const LEvent &triggeringEvent, Int32 x, Int32 y);

    /**
     * @brief Notifies a change of the title string.
     *
     * @see title()
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp titleChanged
     */
    virtual void titleChanged();

    /**
     * @brief Notifies a change of the App ID string.
     *
     * @see appId()
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp appIdChanged
     */
    virtual void appIdChanged();

    /**
     * @brief Notifies a change of the client's preferred decoration mode.
     *
     * @see preferredDecorationMode()
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp preferredDecorationModeChanged
     */
    virtual void preferredDecorationModeChanged();
/// @}

private:
    friend class LCompositor;
    friend class LToplevelResizeSession;
    friend class Protocols::XdgShell::RXdgSurface;
    friend class Protocols::XdgShell::RXdgToplevel;
    friend class Protocols::XdgDecoration::RXdgToplevelDecoration;

    enum Flags : UInt32
    {
        ForceRemoveActivatedFlag    = static_cast<UInt32>(1) << 0,
        HasPendingInitialConf       = static_cast<UInt32>(1) << 1,
        HasSizeOrStateToSend        = static_cast<UInt32>(1) << 2,
        HasDecorationModeToSend     = static_cast<UInt32>(1) << 4,
        HasBoundsToSend             = static_cast<UInt32>(1) << 5,
        HasCapabilitiesToSend       = static_cast<UInt32>(1) << 6,
        closedSent                  = static_cast<UInt32>(1) << 7,
    };

    void handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin) override;
    void sendPendingConfiguration() noexcept;
    void fullAtomsUpdate();
    void partialAtomsUpdate();
    void updateSerial() noexcept;
    void reset() noexcept;

    void setTitle(const char *title)
    {
        if (title)
            m_title = title;
        else
            m_title.clear();

        titleChanged();
    }

    void setAppId(const char *appId)
    {
        if (appId)
            m_appId = appId;
        else
            m_appId.clear();

        appIdChanged();
    }

    Atoms &currentAtoms() noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    Atoms &pendingAtoms() noexcept
    {
        return m_atoms[1 - m_currentAtomsIndex];
    }

    Atoms m_atoms[2];
    UInt8 m_currentAtomsIndex { 0 };

    LBitset<Flags> m_flags { HasPendingInitialConf };

    std::string m_appId;
    std::string m_title;

    LBitset<State> m_supportedStates { Activated | Maximized | Fullscreen | Resizing };

    LWeak<Protocols::XdgDecoration::RXdgToplevelDecoration> m_xdgDecorationRes;
    DecorationMode m_preferredDecorationMode { NoPreferredMode };

    mutable LToplevelResizeSession m_resizeSession { this };
    mutable LToplevelMoveSession m_moveSession { this };

    LWeak<LOutput> m_exclusiveOutput;
    LMargins m_extraGeometry;

    Configuration m_pendingConfiguration, m_lastACKConfiguration;
    std::list<Configuration> m_sentConfigurations;
};

#endif // LTOPLEVELROLE_H
