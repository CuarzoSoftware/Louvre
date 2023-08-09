#ifndef LTOPLEVELROLE_H
#define LTOPLEVELROLE_H
#undef None

#include <LBaseSurfaceRole.h>
#include <LSize.h>
#include <LRect.h>

using namespace Louvre::Protocols;

/*!
 * @brief Toplevel role for surfaces
 *
 * The LToplevelRole class is a role for surfaces used by clients to display application windows that have
 * a title and buttons to close, minimize and maximize.\n
 *
 * <center><img height="250px" src="https://lh3.googleusercontent.com/PvNaxLgkjaPryJ8W_P3bDOccQu1m-zNEcI7aH_R8WggzylV5LQZtuzLTUSImThDI8IVsAI9DERF4cwvSqPAEAyjsihHuPCQlZAbvu33iMC2iXvpTswZC3RBNDyKm1YEWDnnKeCn2Qw=w2400"></center>
 *
 * The Toplevel role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_toplevel) protocol.
 * The Wayland protocol also provides its own Toplevel role but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LToplevelRole : public LBaseSurfaceRole
{
public:

    struct Params;

    /*!
     * @brief LToplevelRole class constructor.
     *
     * @param params Internal library parameters provided in the virtual LCompositor::createToplevelRoleRequest() constructor.
     */
    LToplevelRole(Params *params);

    /*!
     * @brief LToplevelRole class destructor.
     *
     * Invoked after LCompositor::destroyToplevelRoleRequest().
     */
    virtual ~LToplevelRole();

    LToplevelRole(const LToplevelRole&) = delete;
    LToplevelRole& operator= (const LToplevelRole&) = delete;

    /*!
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

    /*!
     * @brief Toplevel states bitfield
     */
    typedef UInt32 States;

    /*!
     * @brief Flags indicating the possible states of a Toplevel.
     */
    enum State : States
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
        /// Decorations are drawn by the client
        ClientSide = 1,

        /// Decorations are drawn by the compositor
        ServerSide = 2
    };

    /*!
     * @brief Configures the Toplevel.
     *
     * Requests to change the states of the Toplevel while keeping its current size.\n
     *
     * @param stateFlags Union of Toplevel states defined in #States.
     */
    void configure(States stateFlags);

    /*!
     * @brief Configures the Toplevel.
     *
     * Requests to change the size and states of the Toplevel.\n
     * The size refers to its window geometry (ignoring its decoration).
     *
     * @param size Requested size. Passing (0,0) allows the client to decide the size.
     * @param stateFlags Union of Toplevel states defined in #States.
     */
    void configure(const LSize &size, States stateFlags);

    /*!
     * @brief Configures the Toplevel.
     *
     * Requests to change the size and states of the Toplevel.\n
     * The size refers to its window geometry (ignoring its decoration).
     *
     * @param width Suggested width. Passing (0,0) allows the client to decide the size.
     * @param height Suggested height. Passing (0,0) allows the client to decide the size.
     * @param stateFlags Union of Toplevel states defined in #States.
     */
    void configure(Int32 width, Int32 height, States stateFlags);

    /*!
     * @brief Closes the Toplevel.
     *
     * Requests to close the Toplevel (equivalent to pressing the close button on the window).
     */
    void close() const;

    /**
     * @brief Window geometry in surface coordinates
     *
     * The window geometry is a Toplevel rectangle that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const;

    /**
     * @brief Size during a resizing session
     *
     * Utility function to calculate the Toplevel size during an interactive resizing session.\n
     *
     * @param cursorPosDelta Initial pointer position minus the current one
     * @param initialSize Initial Toplevel position
     * @param edge Edge or corner used in the resizing
     *
     * @returns The Toplevel size given the current parameters
     */
    LSize calculateResizeSize(const LPoint &cursorPosDelta, const LSize &initialSize, ResizeEdge edge);

    /*!
     * @brief Get the application ID associated with the toplevel window.
     *
     * @return A string containing the application ID (e.g., "com.cuarzosoftware.Desk"), can be nullptr.
     */
    const char *appId() const;

    /*!
     * @brief Get the window title of the toplevel.
     *
     * @return A string representing the window title, or nullptr.
     */
    const char *title() const;

    /*!
     * @brief Get the minimum size of the window geometry in surface coordinates.
     *
     * If one of the axis is 0, it means it has no minimum size.
     *
     * @return The minimum size as an LSize object.
     */
    const LSize &minSize() const;

    /*!
     * @brief Get the maximum size of the window geometry in surface coordinates.
     *
     * If one of the axis is 0, it means it has no maximum size.
     *
     * @return The maximum size as an LSize object.
     */
    const LSize &maxSize() const;

    /*!
     * @brief Check if the toplevel window is maximized.
     *
     * @return True if the toplevel is maximized; otherwise, false.
     */
    bool maximized() const;

    /*!
     * @brief Check if the toplevel window is in fullscreen mode.
     *
     * @return True if the toplevel is in fullscreen mode; otherwise, false.
     */
    bool fullscreen() const;

    /*!
     * @brief Check if the toplevel window is currently active.
     *
     * @return True if the toplevel is active; otherwise, false.
     */
    bool activated() const;

    /*!
     * @brief Get the flags representing the current states of the toplevel window.
     *
     * @return The states of the toplevel window as a #States bitfield.
     */
    States states() const;

    /*!
     * @brief Request the client to change its decoration mode.
     *
     * The client may choose to ignore the request. Must be followed by a configure() event.
     *
     * @param mode The desired decoration mode for the toplevel window.
     */
    void setDecorationMode(DecorationMode mode);

    /*!
     * @brief Get the current decoration mode of the toplevel window.
     *
     * @return The current decoration mode.
     */
    DecorationMode decorationMode() const;

    /*!
     * @brief Get the preferred decoration mode set by the client.
     *
     * If 0 is returned, it means the client has no preferred mode.
     *
     * @return The preferred decoration mode as a UInt32 value.
     */
    UInt32 preferredDecorationMode() const;

    /*!
     * @brief Get the xdg_toplevel resource associated with the toplevel window.
     *
     * @return A pointer to the xdg_toplevel resource.
     */
    XdgShell::RXdgToplevel *xdgToplevelResource() const;

    /*!
     * @brief Get the xdg_surface resource associated with the toplevel window.
     *
     * @return A pointer to the xdg_surface resource.
     */
    XdgShell::RXdgSurface *xdgSurfaceResource() const;

/// @name Virtual Methods
/// @{
    /*!
     * @brief Position of the surface according to the role.
     *
     * The default implementation of rolePos() positions the Toplevel at the position assigned by the compositor
     * with LSurface::setPos() minus the (x, y) coords of its window geometry.
     *
     * Reimplement this virtual method if you wish to define your own logic for positioning the Toplevel.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /*!
     * @brief Request to start an interactive move session
     *
     * Reimplement this virtual method if you wish to be notified when the client wishes to start an interactive move session.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startMoveRequest
     */
    virtual void startMoveRequest();

    /*!
     * @brief Request to start an interactive resize session
     *
     * Reimplement this virtual method if you want to be notified when the client wants to start an interactive resize session.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp startResizeRequest
     */
    virtual void startResizeRequest(ResizeEdge edge);

    /*!
     * @brief Request for configuration
     *
     * Reimplement this virtual method if you want to be notified when the client wants the compositor to configure the Toplevel.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

    /*!
     * @brief Request to maximize
     *
     * Reimplement this virtual method if you want to be notified when the client wants to maximize the Toplevel.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp setMaximizedRequest
     */
    virtual void setMaximizedRequest();

    /*!
     * @brief Request to unmaximize
     *
     * Reimplement this virtual method if you want to be notified when the client wants to unmaximize the Toplevel.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetMaximizedRequest
     */
    virtual void unsetMaximizedRequest();

    /*!
     * @brief Maximized state change
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its maximized state.\n
     * You can know if the Toplevel is maximized with maximized().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp maximizedChanged
     */
    virtual void maximizedChanged();

    /*!
     * @brief Request to minimize
     *
     * Reimplement this virtual method if you want to be notified when the client wants to minimize the Toplevel.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setMinimizedRequest
     */
    virtual void setMinimizedRequest();

    /*!
     * @brief Request to set fullscreen mode
     *
     * Reimplement this virtual method if you want to be notified when the client wants to show the Toplevel in fullscreen mode.
     *
     * @param destOutput Output on which the Toplevel should be shown. If it is nullptr the compositor should choose the output.
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setFullscreenRequest
     */
    virtual void setFullscreenRequest(LOutput *destOutput);

    /*!
     * @brief Request to unset fullscreen mode
     *
     * Reimplement this virtual method if you want to be notified when the client wants to deactivate fullscreen mode.
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp unsetFullscreenRequest
     */
    virtual void unsetFullscreenRequest();

    /*!
     * @brief Change in fullscreen mode state
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its fullscreen mode state.\n
     * You can know if the Toplevel is in fullscreen mode with fullscreen().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp fullscreenChanged
     */
    virtual void fullscreenChanged();

    /*!
     * @brief Request to show a menu
     *
     * Reimplement this virtual method if you want to be notified when the client requests the compositor to show a menu
     * containing options to minimize, maximize and change to fullscreen mode.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp showWindowMenuRequest
     */
    virtual void showWindowMenuRequest(Int32 x, Int32 y);

    /*!
     * @brief Change of active state
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its active state.\n
     * You can check if the Toplevel is active with activated().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp activatedChanged
     */
    virtual void activatedChanged();

    /*!
     * @brief Change of maximum size
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its maximum size.\n
     * You can access the maximum size with maxSize().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp maxSizeChanged
     */
    virtual void maxSizeChanged();

    /*!
     * @brief Change of minimum size
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its minimum size.\n
     * You can access the minimum size with minSize().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp minSizeChanged
     */
    virtual void minSizeChanged();

    /**
     * @brief Title change
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its title.
     * You can access the Toplevel's title with title().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp titleChanged
     */
    virtual void titleChanged();

    /**
     * @brief Application ID change
     *
     * Reimplement this virtual method if you want to be notified when the client changes its application ID.
     * You can access the application ID with appId().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp appIdChanged
     */
    virtual void appIdChanged();

    /**
     * @brief Window geometry change
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its window geometry.
     * You can access the Toplevel's window geometry with windowGeometry().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /**
     * @brief Decoration mode change
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its decoration mode.
     * You can access the Toplevel's decoration mode with decorationMode().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp decorationModeChanged
     */
    virtual void decorationModeChanged();

    /**
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its preferred decoration mode.
     * You can access the Toplevel's preferred decoration mode with preferredDecorationMode().
     *
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp preferredDecorationModeChanged
     */
    virtual void preferredDecorationModeChanged();
/// @}

    LPRIVATE_IMP(LToplevelRole)

    void handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin) override;
};

#endif // LTOPLEVELROLE_H
