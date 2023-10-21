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
 * Popup surfaces are always children of other Popups or Toplevels.
 * They have complex positioning rules defined in their LPositioner instance.
 * The default implementation of rolePos() implements these rules, making it possible to restrict the area where the Popup should be positioned with setPositionerBounds().\n
 *
 * The Popup role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_popup).
 * The Wayland protocol also offers its own Popup role, but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LPopupRole : public LBaseSurfaceRole
{
public:
    struct Params;

    /**
     * @brief Constructor for LPopupRole class.
     * @param params Internal library parameters provided in the LCompositor::createPopupRoleRequest() virtual constructor.
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
     * The window geometry is a rect within the Popup that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const;

    /**
     * @brief Get Positioning Rules for the Popup.
     *
     * This method returns the positioning rules for the Popup.
     *
     * @return The rules governing the Popup's positioning.
     */
    const LPositioner &positioner() const;

    /**
     * @brief Set Positioning Constraints for the Popup.
     *
     * This method is used to define the area within which the Popup can be positioned.
     *
     * @param bounds The constraint rect (x, y, width, height). Providing a rect with zero area deactivates the constraint.
     */
    void setPositionerBounds(const LRect &bounds);

    /**
     * @brief Get the Constraint Area for Popup Positioning.
     *
     * This method returns the constraint area for positioning the Popup.\n
     * You can set this constraint area using setPositionerBounds().
     *
     * @note This constraint area is utilized in the default implementation of rolePos().
     *
     * @return The constraint area for Popup positioning.
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

    /**
     * @brief Configure the Popup.
     *
     * This method instructs the client to set the size and position of the Popup.\n
     * The position is relative to its parent's position, and the size refers to the window geometry of the Popup, excluding its decoration.
     *
     * @param rect The suggested position and size for the configuration.
     */
    void configure(const LRect &rect) const;

    /**
     * @brief Dismiss the Popup.
     *
     * This method dismisses the Popup along with all its children, starting from the topmost and descending downwards.
     */
    void sendPopupDoneEvent();

    /**
     * @brief Check if this Popup is the Topmost.
     *
     * This method checks whether this Popup is the topmost one within a client.
     *
     * @return `true` if it's the topmost Popup, `false` otherwise.
     */
    bool isTopmostPopup() const;

    /// @name Virtual Methods

    /// @{

    /**
     * @brief Position of the Popup surface according to the role.
     *
     * By default, this method returns the position of the Popup according to the rules defined by
     * its LPositioner and within the bounds specified by positionerBounds().
     *
     * If you need to customize the logic for positioning the Popup, you can reimplement this virtual method.
     *
     * @note Additionally, this method computes the size that the Popup should have to remain unconstrained and stores it in LPositioner::unconstrainedSize().
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Handle Popup Window Geometry Changes.
     *
     * This virtual method is triggered when the Popup's window geometry changes, typically in response to a configure() event.\n
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /**
     * @brief Request to initiate a keyboard grab.
     *
     * This virtual method is triggered when the Popup requests to initiate a keyboard grab for its surface.\n
     * Override this virtual method if you need to be notified when such a grab request occurs.
     *
     * @note During a keyboard grab, no other surfaces can acquire keyboard focus.
     *       Additionally, if the surface undergoing the grab is destroyed, then the keyboard grab is transferred to its parent surface.
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp grabSeatRequest
     *
     * @param seatGlobal A pointer to the client Wayland seat resource associated with the grab request.
     */
    virtual void grabSeatRequest(Protocols::Wayland::GSeat *seatGlobal);

    /**
     * @brief Handle configuration requests for the Popup.
     *
     * Override this virtual method if you wish to receive notifications when the Popup requests configuration.
     * Clients typically request configuration when they want the Popup to be mapped.
     *
     * @note The client expects the compositor to configure the Popup according to the rules defined by its positioner().
     *       Louvre handles this for you through the rolePos() method.
     *
     * @see rolePos()
     *
     * #### Default Implementation
     *
     * The default implementation restricts the positioning area of the Popup to the output where the cursor is located.
     * It then calculates the Popup's position based on its LPositioner rules and subtracts the parent surface's position
     * to obtain the local position relative to its parent.
     * Finally, it configures the Popup using the calculated position and size.
     *
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
