#ifndef LSUBSURFACEROLE_H
#define LSUBSURFACEROLE_H

#include <LBaseSurfaceRole.h>

/*!
 * @brief Subsurface role for surfaces
 * 
 * The LSubsurfaceRole class is a role for surfaces that allows them to be positioned relatively to their parents.\n
 * Subsurfaces are always children of other surfaces.\n
 * Their position is given by that of their parent plus the offset given by localPosS() or localPosC().\n
 *
 * @section Modes
 *
 * Subsurfaces can work in two modes:
 *
 * ### Synchronous Mode
 *
 * In this mode, changes to the subsurface, such as its position or commit, are applied exclusively when its parent performs a commit.\n
 * This mode is used by clients to synchronize the animation or movement of multiple subsurfaces.\n
 * The default library keeps track of changes and applies them when the parent performs a commit.\n
 *
 * ### Asynchronous Mode
 *
 * In this mode, subsurface changes are applied independently of the commits of its parent.\n
 * This mode is useful for example to perform efficient composition in video players where the user interface is independent
 * of the rect where the video is displayed.\n
 * The Subsurface role is part of the [Wayland protocol](https://wayland.app/protocols/wayland#wl_subsurface).
 */

class Louvre::LSubsurfaceRole : public LBaseSurfaceRole
{
public:

    struct Params;

    /*!
     * @brief Constructor for the LSubsurfaceRole class.
     *
     * @param params Internal parameters of the library passed in the virtual constructor LCompositor::createSubsurfaceRoleRequest().
     */
    LSubsurfaceRole(Params *params);

    /*!
     * @brief Destructor for the LSubsurfaceRole class.
     *
     * Invoked after LCompositor::destroySubsurfaceRoleRequest().
     */
    virtual ~LSubsurfaceRole();

    LSubsurfaceRole(const LSubsurfaceRole&) = delete;
    LSubsurfaceRole& operator= (const LSubsurfaceRole&) = delete;

    /*!
     * @brief Current mode.
     *
     * @returns true if in synchronous mode, false otherwise.
     */
    bool isSynced() const;

    /*!
    * @brief Offset in surface coordinates.
    *
    * The offset relative to the top-left corner of the parent surface in surface coordinates.
    */
    const LPoint &localPosS() const;

    /*!
    * @brief Offset in compositor coordinates.
    *
    * The offset relative to the top-left corner of the parent surface in compositor coordinates.
    */
    const LPoint &localPosC() const;

/// @name Virtual Methods
/// @{

    /*!
     * @brief Position of the subsurface according to the role.
     *
     * The default implementation of rolePosC() positions the subsurface relative to its parent by adding the offset
     * given by localPosC().\n
     * Reimplement this virtual method if you want to define your own logic for positioning the subsurface.
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp rolePosC
     */
    virtual const LPoint &rolePosC() const override;

    /*!
     * @brief Change of offset.
     *
     * Reimplement this virtual method if you want to be notified when the offset changes.
     *
     * #### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp localPosChanged
     */
    virtual void localPosChanged();

    /*!
     * @brief Change of mode.
     *
     * Reimplement this virtual method if you want to be notified when the subsurface changes mode.
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp syncModeChanged
     */
    virtual void syncModeChanged();

    /*!
     * @brief Place above.
     *
     * Reimplement this virtual method if you want to be notified when the sub-surface requests to be placed above the **sibling** surface.\n
     * The library automatically maintains the hierarchical order in the list of surfaces of the compositor (LCompositor::surfaces()).
     *
     * #### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedAbove(LSurface *sibiling);

    /*!
     * @brief Place below.
     *
     * Reimplement this virtual method if you want to be notified when the subsurface requests to be placed below the **sibling** surface.\n
     * The library automatically maintains the hierarchical order in the list of surfaces of the compositor (LCompositor::surfaces()).
     *
     * ### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedBelow(LSurface *sibiling);
/// @}

    LPRIVATE_IMP(LSubsurfaceRole)

    bool acceptCommitRequest(Protocols::Wayland::RSurface::CommitOrigin origin) override;
    void handleSurfaceCommit() override;
    void handleParentCommit() override;
    void handleParentChange() override;
    void handleParentMappingChange() override;
    void globalScaleChanged(Int32 oldScale, Int32 newScale) override;
};

#endif // LSUBSURFACEROLE_H
