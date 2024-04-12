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
 * Popup surfaces are always children of other popups or Toplevels.
 * They have complex positioning rules defined in their LPositioner instance.
 * The default implementation of rolePos() implements these rules, making it possible to restrict the area where the popup should be positioned with setPositionerBounds().\n
 *
 * The popup role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_popup).
 * The Wayland protocol also offers its own popup role, but it is considered obsolete and therefore not included in the library.
 */
class Louvre::LPopupRole : public LBaseSurfaceRole
{
public:
    struct Params;

    /**
     * @brief Constructor for LPopupRole class.
     * @param params Internal library parameters provided in the LCompositor::createPopupRoleRequest() virtual constructor.
     */
    LPopupRole(const void *params);

    /**
     * @brief Destructor of LPopupRole class.
     *
     * Invoked after LCompositor::destroyPopupRoleRequest().
     */
    virtual ~LPopupRole();

    /// @cond OMIT
    LCLASS_NO_COPY(LPopupRole)
    /// @endcond

    /**
     * @brief Window geometry in surface coordinates.
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a rect within the popup that excludes its decorations (typically shadows).
     */
    const LRect &windowGeometry() const;

    /**
     * @brief Retrieve the current size in surface coordinates.
     *
     * This size corresponds to the value returned by `windowGeometry().size()`.
     */
    const LSize &size() const;

    /**
     * @brief Get the positioning rules for the popup.
     */
    const LPositioner &positioner() const;

    /**
     * @brief Set the positioning bounds constraints for the popup.
     *
     * This method is used to define the area within which the popup can be positioned.
     *
     * @param bounds The constraint rect (x, y, width, height). Providing a rect with zero area deactivates the constraint.
     */
    void setPositionerBounds(const LRect &bounds);

    /**
     * @brief Get the constraint bounds for popup Positioning.
     *
     * This method returns the constraint bounds for positioning the popup which can be set using setPositionerBounds().
     *
     * @note This is used in the default implementation of rolePos().
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
     * @brief Configure the popup.
     *
     * This method instructs the client to set the size and position of the popup.\n
     * The position is relative to its parent's position, and the size refers to the window geometry of the popup, excluding its decoration.
     *
     * To calc the local position you can use `rolePos() - surface()->parent()->pos()` and get the suggested unconstrained size with `positioner().unconstrainedSize()`.
     *
     * @param rect The suggested position and size for the configuration.
     */
    void configure(const LRect &rect) const;

    /**
     * @brief Dismiss the popup.
     *
     * This method closes the popup along with all its children, starting from the topmost and descending downwards.
     */
    void dismiss();

    /**
     * @brief Check if this oopup is the topmost withing the same client.
     *
     * @return `true` if it's the topmost popup, `false` otherwise.
     */
    bool isTopmostPopup() const;

    /// @name Virtual Methods

    /// @{

    /**
     * @brief Position of the popup surface according to the role.
     *
     * By default, this method returns the position of the popup according to the rules defined by
     * its LPositioner and within the bounds specified by positionerBounds().
     *
     * Override this virtual method if you need to customize the logic for positioning the popup.
     *
     * @note Additionally, this method computes the size that the popup should have to remain unconstrained and stores it in LPositioner::unconstrainedSize().
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Handle popup window geometry changes.
     *
     * This virtual method is triggered when the popup's window geometry changes, typically in response to a configure() event.
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp geometryChanged
     */
    virtual void geometryChanged();

    /**
     * @brief Request to initiate a keyboard grab.
     *
     * TODO Update doc
     * This virtual method is triggered when the client requests to initiate a keyboard grab for this popup.\n
     * Override this virtual method if you need to be notified when such a grab request occurs.
     *
     * @note During a keyboard grab, no other surfaces can acquire keyboard focus.
     *       Additionally, if the surface undergoing the grab is destroyed, then the keyboard grab is transferred to its parent surface.
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp grabKeyboardRequest
     *
     * @param seatGlobal A pointer to the client Wayland seat resource associated with the grab request.
     */
    virtual void grabKeyboardRequest(const LEvent &triggeringEvent);

    /**
     * @brief Handle configuration requests for the popup.
     *
     * Override this virtual method if you wish to receive notifications when a popup needs to be configured.
     * Clients request configuration when they want to map the popup.
     *
     * @note The client expects the compositor to configure the popup according to the rules defined by its positioner().
     *       Louvre handles this for you through the rolePos() method.
     *
     * @see rolePos()
     *
     * #### Default Implementation
     *
     * The default implementation restricts the positioning area of the popup to the output where the cursor is located.
     * It then calculates the popup's position based on its LPositioner rules and subtracts the parent surface's position
     * to obtain the local position relative to its parent.
     * Finally, it configures the popup using the calculated position and size.
     *
     * @snippet LPopupRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

    ///@}

    LPRIVATE_IMP_UNIQUE(LPopupRole)

    /// @cond OMIT
    void handleSurfaceCommit(CommitOrigin origin) override;
    /// @endcond
};

#endif // LPOPUPROLE_H
