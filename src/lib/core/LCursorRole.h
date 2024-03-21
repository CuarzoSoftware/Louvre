#ifndef LCURSORROLE_H
#define LCURSORROLE_H

#include <LBaseSurfaceRole.h>

/**
 * @brief Cursor role for surfaces
 *
 * The LCursorRole class is a role for surfaces that allows the compositor to use them as cursors.\n
 * Clients create the role by requesting [set_cursor](https://wayland.app/protocols/wayland#wl_pointer:request:set_cursor)
 * from the [wl_pointer](https://wayland.app/protocols/wayland#wl_pointer) interface of the Wayland protocol.\n
 *
 * @note Louvre automatically invokes the LPointer::setCursorRequest() event when a client with pointer focus wants to assign an LCursorRole as the cursor.
 *       This event is also invoked when the role hotspot changes, so it is not necessary to reimplement this class to handle these changes.
 *
 * @see LPointer::setCursorRequest().
 */
class Louvre::LCursorRole : public LBaseSurfaceRole
{
public:
    struct Params;

    /**
     * @brief Constructor of the LCursorRole class.
     *
     * @param params Internal library parameters passed in the LCompositor::createCursorRoleRequest() virtual constructor.
     */
    LCursorRole(const void *params);

    /**
     * @brief Destructor of the LCursorRole class.
     *
     * Invoked internally by the library after LCompositor::destroyCursorRoleRequest() is called.
     */
    virtual ~LCursorRole();

    /// @cond OMIT
    LCursorRole(const LCursorRole&) = delete;
    LCursorRole& operator= (const LCursorRole&) = delete;
    /// @endcond

    /**
     * @brief Position of the surface given the role.
     *
     * The cursor position given the role is calculated by subtracting the hotspot from the surface position.\n
     * This position is generally not used since the cursor is usually rendered using the LCursor class.\n
     * However, it could be useful in cases where you do not want to use the LCursor class.
     *
     * This method can be reimplemented to change the positioning logic of the surface given the role.
     * #### Default implementation
     * @snippet LCursorRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Notifies a hotspot change.
     *
     * It is recommended to use the LPointer::setCursorRequest() event to listen for hotspot changes instead.
     *
     * #### Default implementation
     * @snippet LCursorRoleDefault.cpp hotspotChanged
     */
    virtual void hotspotChanged();

    /**
     * @brief Cursor hotspot in surface coordinates.
     */
    const LPoint &hotspot() const;

    /**
     * @brief Cursor hotspot in buffer coordinates.
     */
    const LPoint &hotspotB() const;

    LPRIVATE_IMP_UNIQUE(LCursorRole)

    /// @cond OMIT
    virtual void handleSurfaceCommit(CommitOrigin origin) override;
    virtual void handleSurfaceOffset(Int32 x, Int32 y) override;
    /// @endcond
};

#endif // LCURSORROLE_H
