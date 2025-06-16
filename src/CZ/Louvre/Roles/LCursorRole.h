#ifndef LCURSORROLE_H
#define LCURSORROLE_H

#include <LBaseSurfaceRole.h>

/**
 * @brief Cursor role for surfaces.
 *
 * Clients create this role when they want to assign the compositor cursor texture and hotspot.\n
 * Instead of rendering this surface, the LCursor class should be employed, which displays the cursor
 * using hardware composition, which is more efficient.
 *
 * When clients want to assign the cursor, they trigger the LPointer::setCursorRequest(),
 * providing the cursor in the form of an LClientCursor. That class serves as a wrapper for this role,
 * and when assigned to LCursor with LCursor::setCursor(), it automatically updates its buffer,
 * size, hotspot, and visibility.
 *
 * @see LPointer::setCursorRequest()
 */
class Louvre::LCursorRole : public LBaseSurfaceRole
{
public:
    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LCursorRole;

    /**
     * @brief Constructor of the LCursorRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LCursorRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LCursorRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     *
     * @warning The `surface()` handle always remains valid during the destructor call.
     *          However, `LSurface::role()` returns `nullptr` because `LSurface::roleChanged()`
     *          is notified beforehand and requires the role to be valid.
     */
    ~LCursorRole();

    LCLASS_NO_COPY(LCursorRole)

    /**
     * @brief Position of the surface given the role.
     *
     * The cursor position given the role is calculated by subtracting the hotspot from LSurface::pos().\n
     * This position is generally not used since the cursor is usually rendered using the LCursor class.\n
     * However, it could be useful in cases where you do not want to use the LCursor class.
     *
     * #### Default implementation
     * @snippet LCursorRoleDefault.cpp rolePos
     */
    virtual SkIPoint rolePos() const override;

    /**
     * @brief Notifies a hotspot change.
     *
     * #### Default implementation
     * @snippet LCursorRoleDefault.cpp hotspotChanged
     */
    virtual void hotspotChanged();

    /**
     * @brief Cursor hotspot in surface coordinates.
     */
    const SkIPoint &hotspot() const noexcept
    {
        return m_currentHotspot;
    }

    /**
     * @brief Cursor hotspot in buffer coordinates.
     */
    const SkIPoint &hotspotB() const noexcept
    {
        return m_currentHotspotB;
    }

private:
    friend class Protocols::Wayland::RPointer;
    virtual void handleSurfaceCommit(CommitOrigin origin) override;
    virtual void handleSurfaceOffset(Int32 x, Int32 y) override;
    SkIPoint m_currentHotspot, m_pendingHotspotOffset;
    SkIPoint m_currentHotspotB;
};

#endif // LCURSORROLE_H
