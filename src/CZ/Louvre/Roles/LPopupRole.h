#ifndef LPOPUPROLE_H
#define LPOPUPROLE_H

#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>
#include <CZ/Louvre/Roles/LPositioner.h>
#include <CZ/Core/CZEdge.h>
#include <CZ/skia/core/SkRect.h>
#include <list>
#include <memory>

/**
 * @brief Popup role for surfaces
 *
 * The LPopupRole class is a role for surfaces commonly used by clients to display context menus and tooltips.
 *
 * <center><img src="https://lh3.googleusercontent.com/6caGayutKKWqndpd6ogno2lPw8XGELxnFums4gvkWZKOJYO0762yVG3mHLrc1rw63r1eEJabEdW9F5AA2BDTFCtpB_hiPlTY4FkKlHfH1B-2MdLvXCD6RuwZOZvhOl6EhydtsOYGPw=w2400"></center>
 *
 * The popup role is part of the [XDG Shell](https://wayland.app/protocols/xdg-shell#xdg_popup).
 * The Wayland protocol also has its own popup role, but it is considered obsolete and therefore not included in the library.
 */
class CZ::LPopupRole : public LBaseSurfaceRole
{
public:
    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LPopupRole;

    /**
     * @brief Flags indicating which properties changed during a stateChanged() event.
     */
    enum Changes : UInt32
    {
        /// Indicates windowGeometry() changed.
        WindowGeometryChanged   = static_cast<UInt32>(1) << 0,

        /// Indicates localPos() changed.
        LocalPosChanged         = static_cast<UInt32>(1) << 1,

        /// Indicates serial() changed.
        SerialChanged           = static_cast<UInt32>(1) << 2
    };

    /**
     * @brief Configuration parameters sent to the client.
     *
     * @see configureRect()
     * @see pendingConfiguration()
     * @see findConfiguration()
     */
    struct Configuration
    {
        /// Local position and size. See localPos() and windowGeometry().
        SkIRect rect { 0, 0, 0, 0 };

        /// @see LPopupRole::serial() and pendingConfiguration()
        UInt32 serial;
    };

    /**
     * @brief Atomic properties
     *
     * This struct contains all `LPopupRole` properties that should be handled
     * simultaneously during an `atomsChanged()` event.
     *
     * @see atoms()
     */
    struct State
    {
        /// LPopupRole::windowGeometry()
        SkIRect windowGeometry { 0, 0, 0, 0 };

        /// LPopupRole::localPos()
        SkIPoint localPos { 0, 0 };

        /// LPopupRole::serial()
        UInt32 serial;

        bool windowGeometrySet { false };
    };

    /**
     * @brief Constructor of the LPopupRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LPopupRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LPopupRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     *
     * @warning The `surface()` handle always remains valid during the destructor call.
     *          However, `LSurface::role()` returns `nullptr` because `LSurface::roleChanged()`
     *          is notified beforehand and requires the role to be valid.
     */
    ~LPopupRole() noexcept;

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
     * This struct holds the last configuration parameters assigned with configureRect().
     *
     * The pending configuration parameters are sent to the client at most once per Louvre
     * main loop iteration.
     *
     * The pending serial is also updated at most once per loop iteration if a configure
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
     * @brief Configures the popup local position and size.
     *
     * This method instructs the client to set the size and local position of the popup and stores the value in pendingConfiguration().\n
     * The position is relative to its parent's window geometry, and the size does not include the popup decorations, see windowGeometry().
     *
     * @see calculateUnconstrainedRect().
     */
    void configureRect(const SkIRect &rect) const noexcept;

    /**
     * @brief Calculates the size and parent-local position the popup should be configured to in order to be unconstrained.
     *
     * @note Ensure that bounds() is properly set before using this method.
     *
     * @param futureParentPos The future position of the parent surface, or `nullptr` to use the current position.
     *        See LPositioner::hasParentConfigureSerial() and LPositioner::hasParentSize().
     *
     * @return The calculated rect representing the unconstrained size and position.
     */
    SkIRect calculateUnconstrainedRect(const SkIPoint *futureParentPos = nullptr) const noexcept;

    /**
     * @brief Set the positioning bounds constraints for the popup.
     *
     * This method is used to define the area within which the popup must be positioned.
     *
     * @param bounds The constraint rect in compositor-global coordinates. Providing a rect with zero area deactivates the constraint.
     */
    void setBounds(const SkIRect &bounds) noexcept
    {
        m_bounds = bounds;
    }

    /**
     * @brief Gets the constraint bounds.
     *
     * This method returns the constraint bounds for positioning the popup which can be set using setBounds().
     */
    const SkIRect &bounds() const noexcept
    {
        return m_bounds;
    }

    /**
     * @brief Checks which toplevel edges would be constrained if the popup is positioned within the given rect.
     *
     * @param rect The bounds in compositor-global coordinates.
     * @return A bitset indicating the constrained edges.
     */
    CZBitset<CZEdge> constrainedEdges(const SkIRect &rect) const noexcept;

    /**
     * @brief Current atomic properties.
     *
     * This struct contains all the current popup atomic properties, which are updated
     * each time atomsChanged() is triggered.
     *
     * The current properties can also be accessed via aliases such as windowGeometry(), localPos(), serial(), etc.
     */
    const State &state() const noexcept
    {
        return m_current;
    }

    /**
     * @brief Window geometry in surface coordinates
     *
     * <center><img height="300px" src="https://lh3.googleusercontent.com/pw/AIL4fc_le5DeTa6b-yBnChX6YPbkr12gAp38ghVyvsv4SjHCd2L4fTL8agYls0AcGlBeplJyc0FNQCIeb6sR4WbSUyAHM4_LrKLNjhZ0SniRdaSUsjS9IGQ=w2400"></center>
     *
     * The window geometry is a rect within the popups's surface that excludes its decorations (typically shadows).
     *
     * @note This is an alias for Atoms::windowGeometry.
     */
    const SkIRect &windowGeometry() const noexcept
    {
        return state().windowGeometry;
    }

    /**
     * @brief Retrieves the current local position relative to the parent window geometry.
     *
     * Returns the last configured local position acknowledged by the client.\n
     * Initially set to (0,0).
     *
     * @see configureRect()
     *
     * @note This is an alias for Atoms::localPos.
     */
    SkIPoint localPos() const noexcept
    {
        return state().localPos;
    }

    /**
     * @brief Gets the positioning rules for the popup.
     */
    const LPositioner &positioner() const
    {
        return m_positioner;
    }

    const std::list<LPopupRole*> &childPopups() const noexcept { return m_childPopups; };

    /**
     * @brief Dismisses the popup and all its children.
     */
    void dismiss() noexcept;

    /**
     * @brief Check if this oopup is the topmost withing its current tree.
     *
     * @return `true` if it's the topmost popup, `false` otherwise.
     */
    bool isTopmostPopup() const noexcept;

    /**
     * @brief Sets the exclusive output hint.
     *
     * This is an optional hint that can be used to keep a reference of
     * in which LOutput a popup is positioned.
     *
     * The default implementation of `LOutput::paintGL()` uses this information to prevent displaying
     * the given popup on other outputs.
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
    virtual LOutput *exclusiveOutput() const override
    {
        return m_exclusiveOutput;
    }

    /**
     *  @brief [xdg_popup](https://wayland.app/protocols/xdg-shell#xdg_popup) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgPopup *xdgPopupResource() const noexcept;

    /**
     *  @brief [xdg_surface](https://wayland.app/protocols/xdg-shell#xdg_surface) resource from the [XDG Shell](https://wayland.app/protocols/xdg-shell) protocol.
     */
    Protocols::XdgShell::RXdgSurface *xdgSurfaceResource() const noexcept;


    /// @name Virtual Methods
    /// @{

    /**
     * @brief Position of the popup surface according to the role.
     *
     * @see configureRect()
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp rolePos
     */
    SkIPoint extracted() const;
    virtual SkIPoint rolePos() const override;

    /**
     * @brief Handles client requests for configuring the popup.
     *
     * This method is triggered before the client maps the popup surface and
     * each time the client updates the positioner() rules.
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp configureRequest
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
     * @snippet LPopupRoleDefault.cpp atomsChanged
     */
    virtual void stateChanged(CZBitset<Changes> changes, const State &prev);

    /**
     * @brief Request to grab the keyboard.
     *
     * Client request to redirect all keyboard events to the popup. See LKeyboard::setGrab().\n
     * During a keyboard grab, no other surfaces can acquire keyboard focus.\n
     * Additionally, if the surface undergoing the grab is destroyed, the keyboard grab is transferred to its parent surface.
     *
     * @note If the keyboard grab is not set, the popup is dismissed immediately.
     *
     * @param triggeringEvent The input event that triggered the grab request.
     *
     * #### Default Implementation
     * @snippet LPopupRoleDefault.cpp grabKeyboardRequest
     */
    virtual void grabKeyboardRequest(const CZEvent &triggeringEvent);

    ///@}

protected:
    mutable SkIPoint m_rolePos {};

private:
    friend class LCompositor;
    friend class Protocols::XdgShell::RXdgPopup;
    friend class Protocols::XdgShell::RXdgSurface;
    friend class Protocols::LayerShell::RLayerSurface;

    void cacheCommit() noexcept override;
    void applyCommit() noexcept override;
    void applyState(State *pending) noexcept;
    bool setParent(LSurface *parent) noexcept;

    enum Flags : UInt8
    {
        Dismissed               = static_cast<UInt8>(1) << 0,
        HasPendingInitialConf   = static_cast<UInt8>(1) << 1,
        HasPendingReposition    = static_cast<UInt8>(1) << 2,
        HasConfigurationToSend  = static_cast<UInt8>(1) << 3,
        CanBeConfigured         = static_cast<UInt8>(1) << 4
    };

    mutable CZBitset<Flags> m_flags { HasPendingInitialConf };

    State m_current {};
    State m_pending {};
    std::list<std::shared_ptr<State>> m_cache;

    mutable Configuration m_pendingConfiguration {}, m_lastACKConfiguration {};
    std::list<Configuration> m_sentConfs;
    SkIRect m_bounds { 0, 0, 0, 0 };
    LPositioner m_positioner {};
    UInt32 m_repositionToken {};
    CZWeak<LOutput> m_exclusiveOutput;

    std::list<LPopupRole*> m_childPopups;
    std::list<LPopupRole*>::iterator m_parentLink;

    void sendPendingConfiguration() noexcept;
};

#endif // LPOPUPROLE_H
