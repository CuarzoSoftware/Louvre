#ifndef LCURSORROLE_H
#define LCURSORROLE_H

#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>
#include <memory>

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
class CZ::LCursorRole : public LBaseSurfaceRole
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
    ~LCursorRole() noexcept;

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
    SkIPoint hotspot() const noexcept { return m_hotspot; }

    /**
     * @brief Cursor hotspot in buffer coordinates.
     */
    SkIPoint hotspotB() const noexcept { return m_hotspotB; }

private:
    friend class Protocols::Wayland::RPointer;
    void applyCommit() noexcept override;
    void cacheCommit() noexcept override;
    void handleSurfaceOffset(Int32 x, Int32 y) override;

    struct State
    {
        SkIPoint offset;
    };

    State m_current {};
    State m_pending {};
    std::list<State> m_cache;

    SkIPoint m_hotspot {};
    SkIPoint m_hotspotB {};
    std::shared_ptr<LRoleCursorSource> m_cursor;
};

#endif // LCURSORROLE_H
