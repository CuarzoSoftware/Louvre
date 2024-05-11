#ifndef LDNDICONROLE_H
#define LDNDICONROLE_H

#include <LBaseSurfaceRole.h>
#include <memory>

/**
 * @brief Drag & drop icon role for surfaces
 * @ingroup roles
 *
 * The LDNDIconRole class is a role for surfaces that allows the compositor to use them as icons for drag & drop sessions.\n
 * Clients create the role through the [start_drag](https://wayland.app/protocols/wayland#wl_data_device:request:start_drag)
 * request from the Wayland [wl_data_device](https://wayland.app/protocols/wayland#wl_data_device) protocol interface.\n
 * The LDNDIconRole role used in a drag & drop session can be accessed from LDNDManager::icon().\n
 *
 * <center><IMG WIDTH="250px" SRC="https://lh3.googleusercontent.com/evKJ2MbTJ42-qFYSP02NPxUULSFpTz3oBSqn6RvR20u_r5wvgJpHF6o-3Zg7aWgNBhrkIsM8iNWiQQHxPjvGml9zDB2wwNwWK0scqTsHpLIbxMqYv60afSruzbWNBCDZaGI_y77eRA=w2400"></center>
 *
 * @see LDNDManager::startDragRequest()
 */

class Louvre::LDNDIconRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LDNDIconRole;

    /**
     * @brief Constructor of the LDNDIconRole class.
     *
     * @param params Internal library parameters passed in the LCompositor::createDNDIconRoleRequest() virtual constructor.
     */
    LDNDIconRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LDNDIconRole class.
     *
     * Invoked internally by the library after LCompositor::destroyDNDIconRoleRequest() is called.
     */
    virtual ~LDNDIconRole();

    /// @cond OMIT
    LDNDIconRole(const LDNDIconRole&) = delete;
    LDNDIconRole& operator= (const LDNDIconRole&) = delete;
    /// @endcond

    /**
     * @brief Notify a hotspot change.
     *
     * Reimplement this virtual method if you want to be notified when the icon hotspot changes.
     *
     * #### Default implementation
     * @snippet LDNDIconRoleDefault.cpp hotspotChanged
     */
    virtual void hotspotChanged() const;

    /**
     * @brief Position of the surface given the role.
     *
     * The position of the icon given the role is calculated by subtracting the hotspot from the surface position.\n
     *
     * This method can be reimplemented to change the positioning logic of the surface given the role.
     *
     * #### Default implementation
     * @snippet LDNDIconRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Hotspot of the drag & drop icon in surface coordinates.
     */
    const LPoint &hotspot() const;

    /**
     * @brief Hotspot of the drag & drop icon in buffer coordinates.
     */
    const LPoint &hotspotB() const;

    LPRIVATE_IMP_UNIQUE(LDNDIconRole)

    /// @cond OMIT
    virtual void handleSurfaceOffset(Int32 x, Int32 y) override;
    virtual void handleSurfaceCommit(CommitOrigin origin) override;
    /// @endcond
};

#endif // LDNDICONROLE_H
