#ifndef LTOPLEVELROLE_H
#define LTOPLEVELROLE_H
#undef None

#include <LBaseSurfaceRole.h>
#include <LSize.h>
#include <LRect.h>

/**
 * @brief Toplevel role for surfaces
 *
 * The LToplevelRole class is a role for surfaces used by clients to display application windows that have
 * a title and buttons to close, minimize and maximize.\n
 *
 * <center><img height="250px" src="https://lh3.googleusercontent.com/PvNaxLgkjaPryJ8W_P3bDOccQu1m-zNEcI7aH_R8WggzylV5LQZtuzLTUSImThDI8IVsAI9DERF4cwvSqPAEAyjsihHuPCQlZAbvu33iMC2iXvpTswZC3RBNDyKm1YEWDnnKeCn2Qw=w2400"></center>
 *
 * The toplevel role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_toplevel) protocol.
 * The Wayland protocol also provides its own toplevel role but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LToplevelRole : public LBaseSurfaceRole
{
public:

    struct Params;

    /**
     * @brief LToplevelRole class constructor.
     *
     * @param params Internal library parameters provided in the virtual LCompositor::createToplevelRoleRequest() constructor.
     */
    LToplevelRole(const void *params);

    /**
     * @brief LToplevelRole class destructor.
     *
     * Invoked after LCompositor::destroyToplevelRoleRequest().
     */
    virtual ~LToplevelRole();

    /// @cond OMIT
    LToplevelRole(const LToplevelRole&) = delete;
    LToplevelRole& operator= (const LToplevelRole&) = delete;
    /// @endcond

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
     * @brief Toplevel state bitfield
     */
    typedef UInt32 StateFlags;

    /**
     * @brief Flags indicating the possible states of a Toplevel.
     */
    enum State : StateFlags
    {
        /// No state
        NoState     = 0,

        /// Maximized
        Maximized   = 1,

        /// Fullscreen mode
        Fullscreen  = 2,

        /// In interactive resizing
        Resizing    = 4,

        /// Activated (its decorations stand out from others)
        Activated   = 8,

        /// Tiled left (since 2)
        TiledLeft   = 16,

        /// Tiled right (since 2)
        TiledRight  = 32,

        /// Tiled top (since 2)
        TiledTop    = 64,

        /// Tiled bottom (since 2)
        TiledBottom = 128
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
     * @brief Configures the Toplevel.
     *
     * Requests to change the state flags of the toplevel while keeping its pending size (pendingSize()).\n
     *
     * @param flags Union of toplevel state flags defined in #State.
     */
    void configure(StateFlags flags);

    /**
     * @brief Configures the Toplevel.
     *
     * Requests to change the size and state flags of the Toplevel.\n
     * The size refers to its window geometry (ignoring its decoration).
     *
     * @param size Requested size. Passing (0,0) allows the client to decide the size.
     * @param flags Union of toplevel state flags defined in #States.
     */
    void configure(const LSize &size, StateFlags flags);

    /**
     * @brief Configures the Toplevel.
     *
     * Requests to change the size and state flags of the Toplevel.\n
     * The size refers to its window geometry (ignoring its decoration).
     *
     * @param width Suggested width. Passing (0,0) allows the client to decide the size.
     * @param height Suggested height. Passing (0,0) allows the client to decide the size.
     * @param flags Union of toplevel state flags defined in #States.
     */
    void configure(Int32 width, Int32 height, StateFlags flags);

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
     * @brief Retrieve the current size in surface coordinates.
     *
     * This size corresponds to the value returned by `windowGeometry().size()`.
     */
    const LSize &size() const;

    /**
     * @brief Retrieve the size of the last configure() event.
     *
     * This size corresponds to the size of the most recent configure() event.
     * When a configuration event is acknowledged, this size is updated to match the current size (size()).
     */
    const LSize &pendingSize() const;

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
     * @brief Update the position of the toplevel during an interactive resizing session.
     *
     * This method should be called each time the toplevel size changes during a resizing session.
     *
     * @see See an example of its use in the default implementation of LToplevelRole::geometryChanged().
     */
    void updateResizingPos();

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
     * @brief Check if the toplevel is being resized.
     *
     * Equivalent to `states() & Resizing`.
     *
     * @return `true` if the toplevel is being resized; otherwise, `false`.
     */
    bool resizing() const;

    /**
     * @brief Check if the toplevel is maximized.
     *
     * Equivalent to `states() & Maximized`.
     *
     * @return `true` if the toplevel is maximized; otherwise, `false`.
     */
    bool maximized() const;

    /**
     * @brief Check if the toplevel is in fullscreen mode.
     *
     * Equivalent to `states() & Fullscreen`.
     *
     * @return `true` if the toplevel is in fullscreen mode; otherwise, `false`.
     */
    bool fullscreen() const;

    /**
     * @brief Check if the toplevel is currently active.
     *
     * Equivalent to `states() & Activated`.
     *
     * @return `true` if the toplevel is active; otherwise, `false`.
     */
    bool activated() const;

    /**
     * @brief Get the flags representing the current states of the toplevel.
     */
    StateFlags states() const;

    /**
     * @brief Retrieve the state flags of the last configure() event.
     *
     * Once a configuration event is acknowledged, these flags are updated to match the current states (states()).
     */
    StateFlags pendingStates() const;

    /**
     * @brief Request the client to change its decoration mode.
     *
     * The client may choose to ignore the request. Must be followed by a configure() event.
     *
     * @note LToplevelRole::NoPreferredMode is not a valid decoration mode.
     *
     * @param mode The desired decoration mode for the toplevel window.
     */
    void setDecorationMode(DecorationMode mode);

    /**
     * @brief Get the current decoration mode of the toplevel window.
     *
     * @return The current decoration mode.
     */
    DecorationMode decorationMode() const;

    /**
     * @brief Get the preferred decoration mode set by the client.
     *
     * @return The preferred decoration mode value.
     */
    DecorationMode preferredDecorationMode() const;

    /**
     *  @brief [xdg_toplevel](https://wayland.app/protocols/xdg-shell#xdg_toplevel) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgToplevel *xdgToplevelResource() const;

    /**
     *  @brief [xdg_surface](https://wayland.app/protocols/xdg-shell#xdg_surface) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgSurface *xdgSurfaceResource() const;

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
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startMoveRequest
     */
    virtual void startMoveRequest();

    /**
     * @brief Request to start an interactive resize session
     *
     * Override this virtual method if you want to be notified when the client wants to start an interactive resize session.
     *
     * @see LPointer::startResizingToplevel()
     *
     * @param edge Which edge or corner is being dragged.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startResizeRequest
     */
    virtual void startResizeRequest(ResizeEdge edge);

    /**
     * @brief Resizing state change
     *
     * Override this virtual method if you want to be notified when the toplevel changes its resizing state.\n
     * You can know if the toplevel is being resized with the resizing() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp resizingChanged
     */
    virtual void resizingChanged();

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
     * @brief Maximized state change
     *
     * Override this virtual method if you want to be notified when the toplevel changes its maximized state.\n
     * You can know if the toplevel is maximized with the maximized() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp maximizedChanged
     */
    virtual void maximizedChanged();

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
    virtual void showWindowMenuRequest(Int32 x, Int32 y);

    /**
     * @brief Change in fullscreen mode state
     *
     * Override this virtual method if you want to be notified when the toplevel changes its fullscreen mode state.\n
     * You can know if the toplevel is in fullscreen mode with fullscreen().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp fullscreenChanged
     */
    virtual void fullscreenChanged();

    /**
     * @brief Change of active state
     *
     * Override this virtual method if you want to be notified when the toplevel changes its active state.\n
     * You can check if the toplevel is active with the activated() property.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp activatedChanged
     */
    virtual void activatedChanged();

    /**
     * @brief Notification of state flag changes
     *
     * Override this virtual method if you wish to receive notifications when the Toplevel's state flags change.
     *
     * You can access the current state flags using the states() property.
     *
     * @note When this method is called, the pendingStates() property is updated to match the value of states().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp stateChanged
     */
    virtual void statesChanged();

    /**
     * @brief Change of maximum size
     *
     * Override this virtual method if you want to be notified when the toplevel changes its maximum size.\n
     * You can access the maximum toplevel size with the maxSize() property.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp maxSizeChanged
     */
    virtual void maxSizeChanged();

    /**
     * @brief Change of minimum size
     *
     * Override this virtual method if you want to be notified when the toplevel changes its minimum size.\n
     * You can access the minimum size with the minSize() property.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp minSizeChanged
     */
    virtual void minSizeChanged();

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
     * @brief Window geometry change
     *
     * Override this virtual method if you want to be notified when the toplevel changes its window geometry.
     * You can access the Toplevel's window geometry with the windowGeometry() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /**
     * @brief Decoration mode change
     *
     * Override this virtual method if you want to be notified when the toplevel changes its decoration mode.
     * You can access the Toplevel's decoration mode with the decorationMode() property.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp decorationModeChanged
     */
    virtual void decorationModeChanged();

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

    /// @cond OMIT
    void handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    /// @endcond
};

#endif // LTOPLEVELROLE_H
