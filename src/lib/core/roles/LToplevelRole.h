#ifndef LTOPLEVELROLE_H
#define LTOPLEVELROLE_H
#undef None

#include <LBaseSurfaceRole.h>
#include <LBitset.h>
#include <LSize.h>
#include <LRect.h>
#include <string>
#include <memory>

/**
 * @brief Toplevel role for surfaces
 * @ingroup roles
 *
 * The toplevel surface role represents the thypical desktop windows that usually have a title and buttons to close, minimize and maximize.\n
 *
 * <center><img height="250px" src="https://lh3.googleusercontent.com/PvNaxLgkjaPryJ8W_P3bDOccQu1m-zNEcI7aH_R8WggzylV5LQZtuzLTUSImThDI8IVsAI9DERF4cwvSqPAEAyjsihHuPCQlZAbvu33iMC2iXvpTswZC3RBNDyKm1YEWDnnKeCn2Qw=w2400"></center>
 *
 * The toplevel role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_toplevel) protocol.
 * 
 * @note The Wayland protocol also provides its own toplevel role but it is considered obsolete and therefore not included in Louvre.
 */
class Louvre::LToplevelRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LToplevelRole;

    // TODO
    void setExclusiveOutput(LOutput *output) noexcept;
    virtual LOutput *exclusiveOutput() const override;

    void setPrevRect(const LRect &rect) noexcept
    {
        m_prevRect = rect;
    }

    const LRect &prevRect() const noexcept
    {
        return m_prevRect;
    }

    /**
     * @brief Edge constraint when resizing a Toplevel
     */
    enum ResizeEdgeSize : Int32
    {
        /// Disables the constraint on the specified edge.
        EdgeDisabled = std::numeric_limits<Int32>::min()
    };

    /**
     * @brief LToplevelRole class constructor.
     *
     * @param params Internal library parameters provided in the virtual LCompositor::createToplevelRoleRequest() constructor.
     */
    LToplevelRole(const void *params) noexcept;

    /**
     * @brief LToplevelRole class destructor.
     *
     * Invoked after LCompositor::destroyToplevelRoleRequest().
     */
    ~LToplevelRole();

    LCLASS_NO_COPY(LToplevelRole)

    /**
     * @brief Resizing border/corner.
     */
    enum ResizeEdge : UInt32
    {
        /// No edge
        NoEdge = 0,

        /// Top edge
        Top = 1,

        /// Bottom edge
        Bottom = 2,

        /// Left edge
        Left = 4,

        /// Top left corner
        TopLeft = 5,

        /// Bottom left corner
        BottomLeft = 6,

        /// Right edge
        Right = 8,

        /// Top right corner
        TopRight = 9,

        /// Bottom right corner
        BottomRight = 10
    };

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

    /// Compositor capabilities TODO
    enum Capabilities : UInt8
    {
        WindowMenuCap  = static_cast<UInt8>(1) << 0,
        MaximizeCap    = static_cast<UInt8>(1) << 1,
        FullscreenCap  = static_cast<UInt8>(1) << 2,
        MinimizeCap    = static_cast<UInt8>(1) << 3
    };

    /**
     * @brief Represents configuration parameters sent to the client.
     */
    struct Configuration
    {
        /**
         * @brief Size of the toplevel.
         */
        LSize size;

        /**
         * @brief Flags indicating the toplevel states.
         */
        LBitset<State> state;

        DecorationMode decorationMode { ClientSide };

        /**
         * @brief Serial number of the configuration.
         */
        UInt32 serial;
    };

    enum ConfigurationChanges : UInt8
    {
        WindowGeometryChanged     = static_cast<UInt8>(1) << 0,
        StateChanged              = static_cast<UInt8>(1) << 1,
        DecorationModeChanged     = static_cast<UInt8>(1) << 2,
        MinSizeChanged            = static_cast<UInt8>(1) << 3,
        MaxSizeChanged            = static_cast<UInt8>(1) << 4,
    };


    const Configuration *findConfiguration(UInt32 serial) const noexcept;


    /// The states supported by the toplevel
    /// Activated, Maximized, Fullscreen and Resizing are always supported
    /// If any of the tiled states is supported all are supported.
    /// The Suspended state support is independent of other states.
    LBitset<State> supportedStates() const noexcept;

    /**
     * @brief Retrieves the last configuration acknowledged by the client.
     */
    const Configuration &current() const noexcept;

    /**
     * @brief Retrieves the most recent configuration parameters not yet acknowledged by the client.
     * @note When no configuration is sent, it should be equal to the current configuration.
     */
    const Configuration &pending() const noexcept;

    /**
     * @brief Retrieves the configuration preceding the current one.
     */
    const Configuration &previous() const noexcept;

    /**
     * @brief Sets the state flags for the pending configuration.
     * @param flags The state flags to set.
     */
    void configureState(LBitset<State> flags) noexcept;

    /**
     * @brief Sets the size for the pending configuration.
     * @param size The size to set.
     */
    void configureSize(const LSize &size) noexcept;

    /**
     * @brief Sets the size for the pending configuration.
     * @param width The width of the size to set.
     * @param height The height of the size to set.
     */
    void configureSize(Int32 width, Int32 height) noexcept;

    // TODO
    void configureDecorationMode(DecorationMode mode) noexcept;

    DecorationMode decorationMode() const noexcept;
    bool activated() const noexcept;
    bool maximized() const noexcept;
    bool fullscreen() const noexcept;
    bool tiled() const noexcept;
    bool resizing() const noexcept;

    /**
     * @brief Closes the Toplevel.
     *
     * Requests to close the toplevel (equivalent to pressing the close button on the window).
     */
    void close() const;

    /**
     * @brief Window geometry in surface coordinates
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a toplevel rectangle that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const;

    /**
     * @brief Size during a resizing session
     *
     * Utility method to calculate the toplevel size during an interactive resizing session.\n
     *
     * @param cursorPosDelta Initial pointer position minus the current one
     * @param initialSize Initial toplevel position
     * @param edge Edge or corner used in the resizing
     *
     * @returns The toplevel size given the current parameters
     */
    LSize calculateResizeSize(const LPoint &cursorPosDelta, const LSize &initialSize, ResizeEdge edge);

    /**
     * @brief Get the application ID associated with the toplevel window.
     *
     * @return A string containing the application ID (e.g., "org.cuarzosoftware.Desk").
     */
    const std::string &appId() const;

    /**
     * @brief Get the window title of the toplevel.
     *
     * @return A string representing the window title.
     */
    const std::string &title() const;

    /**
     * @brief Get the minimum size of the toplevel in surface coordinates.
     *
     * If one of the axis is 0, it means that axis has no expected minimum size.
     *
     * @return The minimum size as a Louvre::LSize object.
     */
    const LSize &minSize() const;

    /**
     * @brief Get the maximum size of the toplevel in surface coordinates.
     *
     * If one of the axis is 0, it means that axis has no expected maximum size.
     *
     * @return The maximum size as a Louvre::LSize object.
     */
    const LSize &maxSize() const;

    /**
     * @brief Check if the provided size falls within the range defined by minSize() and maxSize().
     *
     * @param size The size to be checked.
     * @return `true` if the size is within the specified range, `false` otherwise.
    */
    bool sizeInRange(const LSize &size) const;

    /**
     * @brief Get the preferred decoration mode set by the client.
     *
     * @return The preferred decoration mode value.
     */
    DecorationMode preferredDecorationMode() const;

    bool supportServerSideDecorations() const noexcept;

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
    LToplevelMoveSession &moveSession() const;

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
    LToplevelResizeSession &resizeSession() const;

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
    virtual void startResizeRequest(const LEvent &triggeringEvent, ResizeEdge edge);

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

    // TODO: Triggered whenever the current state, window geometry or decoration mode changes
    virtual void configurationChanged(LBitset<ConfigurationChanges> changes);
    LMargin calculateConstraintsFromOutput(LOutput *output) const noexcept;

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
     * @brief Request to minimize
     *
     * Reimplement this virtual method if you want to be notified when the client wants to minimize the Toplevel.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setMinimizedRequest
     */
    virtual void setMinimizedRequest();

    /**
     * @brief Request to show a menu
     *
     * Override this virtual method if you want to be notified when the client requests the compositor to show a menu
     * containing options to minimize, maximize and change the fullscreen mode.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp showWindowMenuRequest
     */
    virtual void showWindowMenuRequest(const LEvent &triggeringEvent, Int32 x, Int32 y);

    /**
     * @brief Title change
     *
     * Override this virtual method if you want to be notified when the toplevel changes its title.
     * You can access the Toplevel's title with the title() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp titleChanged
     */
    virtual void titleChanged();

    /**
     * @brief Application ID change
     *
     * Override this virtual method if you want to be notified when the client changes its application ID.
     * You can access the application ID with the appId() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp appIdChanged
     */
    virtual void appIdChanged();

    /**
     * Override this virtual method if you want to be notified when the toplevel changes its preferred decoration mode.
     * You can access the Toplevel's preferred decoration mode with the preferredDecorationMode() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp preferredDecorationModeChanged
     */
    virtual void preferredDecorationModeChanged();
/// @}

    LPRIVATE_IMP_UNIQUE(LToplevelRole)
    void handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin) override;
    LRect m_prevRect;
};

#endif // LTOPLEVELROLE_H
