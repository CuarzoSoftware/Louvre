#ifndef LSUBSURFACEROLE_H
#define LSUBSURFACEROLE_H

#include <LBaseSurfaceRole.h>

/**
 * @brief Subsurface role for surfaces
 *
 * LSubsurfaceRole surfaces are always children of other surfaces and are positioned relative to their parent surface.
 * They are meant to be treated and composited as if they were part of their parent surface, for example, for displaying
 * a video frame within a web browser window.
 *
 * The Subsurface role is part of the [Wayland protocol](https://wayland.app/protocols/wayland#wl_subsurface).
 *
 * @section Ordering
 *
 * Clients can request to modify the order in which they are stacked relative to their parent surface or
 * other sibling surfaces. See placedAbove() and placedBelow(). Louvre automatically updates the LCompositor::surfaces()
 * list, keeping the order and also triggering the LSurface::orderChanged() event.
 *
 * @section Modes
 *
 * Subsurfaces can operate in two modes:
 *
 * ### Synchronous Mode
 *
 * In this mode, changes to the subsurface, such as its position, are applied exclusively when its parent performs a commit.
 * This mode is used by clients to synchronize the animation or movement of multiple subsurfaces.
 * The library keeps track of changes and applies them when the parent performs a commit.
 *
 * ### Asynchronous Mode
 *
 * In this mode, subsurface changes are applied independently of the commits of its parent.
 *
 * @note This documentation is for educational purposes only. Louvre handles all of this internally for you.
 */
class Louvre::LSubsurfaceRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LSubsurfaceRole;

    /**
     * @brief Constructor of the LSubsurfaceRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LSubsurfaceRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LSubsurfaceRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LSubsurfaceRole() = default;

    LCLASS_NO_COPY(LSubsurfaceRole)

    /**
     * @brief Current mode.
     *
     * @returns `true` if in synchronous mode, `false` otherwise.
     */
    bool isSynced() const noexcept
    {
        return m_isSynced;
    }

    /**
    * @brief Offset in surface coordinates.
    *
    * The offset relative to the top-left corner of the parent surface in surface coordinates.
    */
    const LPoint &localPos() const noexcept
    {
        return m_currentLocalPos;
    }

/// @name Virtual Methods
/// @{

    /**
     * @brief Position of the subsurface according to the role.
     *
     * The default implementation of rolePos() positions the subsurface relative to its parent rolePos() by adding the offset
     * given by localPos().
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Change of offset.
     *
     * #### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp localPosChanged
     */
    virtual void localPosChanged();

    /**
     * @brief Change of mode.
     *
     * ### Default implementation
     * @snippet LSubsurfaceRoleDefault.cpp syncModeChanged
     */
    virtual void syncModeChanged();

    /**
     * @brief Place above.
     *
     * @note Louvre automatically maintains the hierarchical order of LCompositor::surfaces().
     *
     * #### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedAbove(LSurface *sibling);

    /**
     * @brief Place below.
     *
     * @note Louvre automatically maintains the hierarchical order of LCompositor::surfaces().
     *
     * ### Default Implementation
     * @snippet LSubsurfaceRoleDefault.cpp placedAbove
     */
    virtual void placedBelow(LSurface *sibling);
/// @}

private:
    friend class Protocols::Wayland::RSubsurface;
    bool acceptCommitRequest(LBaseSurfaceRole::CommitOrigin origin) override;
    void handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin) override;
    void handleParentCommit() override;
    void handleParentChange() override;
    void handleParentMappingChange() override;

    LPoint m_currentLocalPos, m_pendingLocalPos;

    bool m_isSynced { true };
    bool m_hasCache { true };
    bool m_hasPendingLocalPos { true };

    LWeak<LSurface> m_pendingPlaceAbove;
    LWeak<LSurface> m_pendingPlaceBelow;
};

#endif // LSUBSURFACEROLE_H
