#ifndef LSUBSURFACEROLE_H
#define LSUBSURFACEROLE_H

#include <LBaseSurfaceRole.h>

/**
 * @brief Subsurface role for surfaces
 *
 * The LSubsurfaceRole class defines a role for surfaces that allows them to be positioned relative to their parent surfaces.
 * They are always children of other surfaces and get their position based on their parent's position plus an offset defined by the localPos() function.
 *
 * The Subsurface role is part of the [Wayland protocol](https://wayland.app/protocols/wayland#wl_subsurface).
 *
 * @section Modes
 *
 * Subsurfaces can work in two modes:
 *
 * ### Synchronous Mode
 *
 * In this mode, changes to the subsurface, such as its position, are applied exclusively when its parent performs a commit.\n
 * This mode is used by clients to synchronize the animation or movement of multiple subsurfaces.\n
 * The library keeps track of changes and applies them when the parent performs a commit.
 *
 * ### Asynchronous Mode
 *
 * In this mode, subsurface changes are applied independently of the commits of its parent.
 *
 */
class Louvre::LSubsurfaceRole : public LBaseSurfaceRole
{
public:

    struct Params;

    /**
     * @brief Constructor for the LSubsurfaceRole class.
     *
     * @param params Internal parameters of the library passed in the virtual constructor LCompositor::createSubsurfaceRoleRequest().
     */
    LSubsurfaceRole(Params *params);

    /**
     * @brief Destructor for the LSubsurfaceRole class.
     *
     * Invoked after LCompositor::destroySubsurfaceRoleRequest().
     */
    virtual ~LSubsurfaceRole();

    /// @cond OMIT
    LSubsurfaceRole(const LSubsurfaceRole&) = delete;
    LSubsurfaceRole& operator= (const LSubsurfaceRole&) = delete;
    /// @endcond

    /**
     * @brief Current mode.
     *
     * @returns `true` if in synchronous mode, `false` otherwise.
     */
    bool isSynced() const;

    /**
    * @brief Offset in surface coordinates.
    *
    * The offset relative to the top-left corner of the parent surface in surface coordinates.
    */
    const LPoint &localPos() const;

/// @name Virtual Methods
/// @{

    /**
     * @brief Position of the subsurface according to the role.
     *
     * The default implementation of rolePos() positions the subsurface relative to its parent by adding the offset
     * given by localPos().\n
     * Reimplement this virtual method if you want to define your own logic for positioning the subsurface.
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Change of offset.
     *
     * Reimplement this virtual method if you want to be notified when the local position changes.
     *
     * #### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp localPosChanged
     */
    virtual void localPosChanged();

    /**
     * @brief Change of mode.
     *
     * Reimplement this virtual method if you want to be notified when the subsurface changes its sync mode.
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp syncModeChanged
     */
    virtual void syncModeChanged();

    /**
     * @brief Place above.
     *
     * Reimplement this virtual method if you want to be notified when the sub-surface is placed above the **sibling** surface.\n
     *
     * @note The library automatically maintains the hierarchical order in the list of surfaces of the compositor (LCompositor::surfaces()).
     *
     * #### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedAbove(LSurface *sibling);

    /**
     * @brief Place below.
     *
     * Reimplement this virtual method if you want to be notified when the subsurface is placed below the **sibling** surface.\n
     *
     * @note The library automatically maintains the hierarchical order in the list of surfaces of the compositor (LCompositor::surfaces()).
     *
     * ### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedBelow(LSurface *sibling);
/// @}

    LPRIVATE_IMP(LSubsurfaceRole)

    /// @cond OMIT
    bool acceptCommitRequest(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    void handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    void handleParentCommit() override;
    void handleParentChange() override;
    void handleParentMappingChange() override;
    /// @endcond
};

#endif // LSUBSURFACEROLE_H
