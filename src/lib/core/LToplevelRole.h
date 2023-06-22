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
        NoEdge = 0,
        Top = 1,
        Bottom = 2,
        Left = 4,
        TopLeft = 5,
        BottomLeft = 6,
        Right = 8,
        TopRight = 9,
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

    /// Flags for Toplevel capabilities of the compositor (since 5)
    enum WmCapabilities : UChar8
    {
        /// Can show custom menus (with options to minimize, maximize, etc)
        WmWindowMenu = 1,

        /// Can show maximized Toplevels
        WmMaximize = 2,

        /// Can show Toplevels in fullscreen mode
        WmFullscreen = 4,

        /// Can minimize Toplevels
        WmMinimize = 8
    };

    /// Compositor capabilities (WmCapabilities) set with setWmCapabilities() (since 5)
    UChar8 wmCapabilities() const;

    /*! Notifies the Toplevel of the compositor's capabilities (since 5)
     *  @param capabilitiesFlags Union of flags defined in #WmCapabilities
     */
    void setWmCapabilities(UChar8 capabilitiesFlags);

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
    void configureC(States stateFlags);

    /*!
     * @brief Configures the Toplevel.
     *
     * Requests to change the size and states of the Toplevel.\n
     * The size refers to its window geometry (ignoring its decoration).
     *
     * @param size Requested size. Passing (0,0) allows the client to decide the size.
     * @param stateFlags Union of Toplevel states defined in #States.
     */
    void configureC(const LSize &size, States stateFlags);

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
    void configureC(Int32 width, Int32 height, States stateFlags);

    /*!
     * @brief Closes the Toplevel.
     *
     * Requests to close the Toplevel (equivalent to pressing the close button on the window).
     */
    void close() const;

    /*!
     * @brief Suggests a Toplevel size.
     *
     * Optional. Sent before initial configuration.
     */
    bool configureBoundsC(const LSize &bounds);

    /*!
     * @brief Suggested size in surface coordinates.
     *
     * Suggested size assigned with configureBoundsC().
     */
    const LSize &boundsS() const;

    /*!
     * @brief Suggested size in compositor coordinates.
     *
     * Suggested size assigned with configureBoundsC().
     */
    const LSize &boundsC() const;

    /**
     * @brief Window geometry in surface coordinates
     *
     * The window geometry is a Toplevel rectangle that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometryS() const;

    /**
     * @brief Window geometry in compositor coordinates
     *
     * The window geometry is a Toplevel rectangle that excludes its decorations (typically shadows).\n
     * Its components (x,y,width,height) are given by:
     *
     * <center><img src="https://lh3.googleusercontent.com/qYH--yLCkb39PBCqAGqNC8l-jL8YuDPCIcUOTaqXyvp0hUr8Rj6Ug8MS7Fb68-XuWOyhsmsOYb5rKaph3hO40w-3J9zRzISuvCRfU5pFf6dVJ8lgbH_JJ2FpkAYijbH0POUyiB7xDw=w2400" width="512px"></center>
     */
    const LRect &windowGeometryC() const;

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

    /// String with the application ID
    const char *appId() const;

    /// Window title
    const char *title() const;

    /// Minimum size of the window geometry in surface coordinates
    const LSize &minSizeS() const;

    /// Maximum size of the window geometry in surface coordinates
    const LSize &maxSizeS() const;

    /// Minimum size of the window geometry in compositor coordinates
    const LSize &minSizeC() const;

    /// Maximum size of the window geometry in compositor coordinates
    const LSize &maxSizeC() const;

    /// Indicates if the Toplevel is maximized
    bool maximized() const;

    /// Indicates if the Toplevel is in fullscreen mode
    bool fullscreen() const;

    /// Indicates if the Toplevel is active
    bool activated() const;

    /// Flags with the current states of the Toplevel (#States)
    States states() const;

    /// Assigns the decoration mode (the client could ignore the request)
    void setDecorationMode(DecorationMode mode);

    /// Current decoration mode
    DecorationMode decorationMode() const;

    /// xdg_toplevel resource
    XdgShell::RXdgToplevel *xdgToplevelResource() const;

    /// xdg_surface resource
    XdgShell::RXdgSurface *xdgSurfaceResource() const;

/// @name Métodos virtuales
/// @{
    /*!
     * @brief Position of the surface according to the role.
     *
     * The default implementation of rolePosC() positions the Toplevel at the position assigned by the compositor
     * with LSurface::setPosC() minus the (x,y) vector of its window geometry.
     *
     * Reimplement this virtual method if you wish to define your own logic for positioning the Toplevel.
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp rolePosC
     */
    virtual const LPoint &rolePosC() const override;

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
     * @brief Request to activate fullscreen mode
     *
     * Reimplement this virtual method if you want to be notified when the client wants to show the Toplevel in fullscreen.
     *
     * @param destOutput Output on which the Toplevel should be shown. If it is nullptr the compositor should choose the output.
     * #### Default implementation
     * @snippet LToplevelRoleDefault.cpp setFullscreenRequest
     */
    virtual void setFullscreenRequest(LOutput *destOutput);

    /*!
     * @brief Request to deactivate fullscreen mode
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
     * @snippet LToplevelRoleDefault.cpp showWindowMenuRequestS
     */
    virtual void showWindowMenuRequestS(Int32 x, Int32 y);

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
     * You can access the maximum size with maxSizeS() or maxSizeC().
     *
     * #### Default Implementation
     * @snippet LToplevelRoleDefault.cpp maxSizeChanged
     */
    virtual void maxSizeChanged();

    /*!
     * @brief Change of minimum size
     *
     * Reimplement this virtual method if you want to be notified when the Toplevel changes its minimum size.\n
     * You can access the minimum size with minSizeS() or minSizeC().
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
     * You can access the Toplevel's window geometry with windowGeometryS() or windowGeometryC().
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
/// @}

    LPRIVATE_IMP(LToplevelRole)

    void handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    void globalScaleChanged(Int32 oldScale, Int32 newScale) override;
};

#endif // LTOPLEVELROLE_H
