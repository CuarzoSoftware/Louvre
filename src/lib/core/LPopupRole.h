#ifndef LPOPUPROLE_H
#define LPOPUPROLE_H

#include <LBaseSurfaceRole.h>
#include <LRect.h>

/**
 * @brief Popup role for surfaces
 *
 * The LPopupRole class is a role for surfaces commonly used by clients to display context menus and tooltips.\n
 * <center><img src="https://lh3.googleusercontent.com/6caGayutKKWqndpd6ogno2lPw8XGELxnFums4gvkWZKOJYO0762yVG3mHLrc1rw63r1eEJabEdW9F5AA2BDTFCtpB_hiPlTY4FkKlHfH1B-2MdLvXCD6RuwZOZvhOl6EhydtsOYGPw=w2400"></center>
 *
 * Popup surfaces are always children of other surfaces (other Popups or Toplevels).
 * They have complex positioning rules defined in their LPositioner instance.
 * The default implementation of rolePos() implements these rules, making it possible to restrict the area where the Popup should be positioned with setPositionerBounds().\n
 * 
 * The Popup role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_popup) protocol.
 * The Wayland protocol also offers its own Popup role, but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LPopupRole : public LBaseSurfaceRole
{
public:
    struct Params;

    /**
     * @brief Constructor for LPopupRole class.
     * @param params Internal library parameters provided in the virtual LCompositor::createPopupRoleRequest() constructor.
     */
    LPopupRole(Params *params);

    /**
     * @brief Destructor of LPopupRole class.
     *
     * Invoked after LCompositor::destroyPopupRoleRequest().
     */
    virtual ~LPopupRole();

    /// @cond OMIT
    LPopupRole(const LPopupRole&) = delete;
    LPopupRole& operator= (const LPopupRole&) = delete;
    /// @endcond

    /**
     * @brief Window geometry in surface coordinates.
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a rectangle of the Popup that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const;

    /**
     * @brief Positioning rules.
     *
     * Rules for positioning the Popup.
     */
    const LPositioner &positioner() const;

    /**
     * @brief Constraints the positioning area of the Popup.
     *
     * Constraints the area where the Popup can be positioned.
     *
     * @param bounds Constraint rectangle (x,y,w,h). Passing a rectangle with area 0 deactivates the constraint.
     */
    void setPositionerBounds(const LRect &bounds);

    /**
     * @brief Popup position constraint area in compositor coordinates.
     *
     * Constraint area of the Popup in compositor coordinates, assigned with setPositionerBounds().
     */
    const LRect &positionerBounds() const;

    /**
     *  @brief [xdg_popup](https://wayland.app/protocols/xdg-shell#xdg_popup) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgPopup *xdgPopupResource() const;

    /**
     *  @brief [xdg_surface](https://wayland.app/protocols/xdg-shell#xdg_surface) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgSurface *xdgSurfaceResource() const;

/// @name Events
/// @{

    /**
     * @brief Configures the Popup.
     *
     * Suggests a size and position for the Popup relative to its parent position.\n
     * The position and size of the rect refers to the window geometry of the Popup (ignoring its decoration).
     *
     * @param rect Suggested position and size.
     */
    void configure(const LRect &rect) const;

    /**
     * @brief Dismisses the popup and its children.
     *
     * This function destroys the popup and all its children, starting from the topmost and proceeding downward.
     */
    void sendPopupDoneEvent();

    /**
     * @brief Checks if this popup is the topmost one within a client.
     *
     * @return `true` if it's the topmost popup, otherwise `false`.
     */
    bool isTopmostPopup() const;

///@}

/// @name Virtual Methods
/// @{
    /**
     * @brief Position of the Popup surface according to the role.
     *
     * The default implementation of rolePos() positions the Popup following the rules of its LPositioner and
     * restricting its area to positionerBounds().
     *
     * Reimplement this virtual method if you want to define your own logic for positioning the Popup.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Geometry change.
     *
     * Change in the geometry of the Popup window accessible with windowGeometry().
     *
     * Reimplement this virtual method if you want to be notified when the Popup changes its window geometry
     * (typically in response to a configure() event).
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /**
     * @brief Request to acquire keyboard focus.
     *
     * Request from the Popup for its surface to acquire keyboard focus.
     *
     * Reimplement this virtual method if you want to be notified when the Popup requests to acquire
     * keyboard focus.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp grabSeatRequest
     */
    virtual void grabSeatRequest(Protocols::Wayland::GSeat *seatGlobal);

    /**
     * @brief Configuration request.
     *
     * Request from a Popup for the compositor to suggest its position and size relative to its parent
     * according to the rules of its LPositioner.
     *
     * Reimplement this virtual method if you want to be notified when the Popup requests to be configured.
     *
     * #### Default implementation
     * @snippet LPopupRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();
///@}

    LPRIVATE_IMP(LPopupRole)

    /// @cond OMIT
    void handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    /// @endcond
};

#endif // LPOPUPROLE_H
