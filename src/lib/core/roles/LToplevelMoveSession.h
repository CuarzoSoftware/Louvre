#ifndef LTOPLEVELMOVESESSION_H
#define LTOPLEVELMOVESESSION_H

#include <LEdge.h>
#include <LMargins.h>
#include <LPoint.h>
#include <functional>
#include <memory>

/**
 * @brief Toplevel move session utility
 *
 * This class serves as a utility for handling interactive move sessions for LToplevelRole surfaces.\n
 * Each LToplevelRole has an associated move session instance LToplevelRole::moveSession().
 *
 * When a client triggers LToplevelRole::startMoveRequest(), the start() method should be invoked,
 * providing the triggering event of the request and an initial drag point, which could be the current position of the pointer
 * (typically obtained via LCursor::pos()) or a touch point if triggered by a touch event. This also adds the session to the
 * LSeat::toplevelMoveSessions() vector.
 *
 * Subsequently, whenever the drag point changes, such as within an LPointer::pointerMoveEvent() or LTouch::touchMoveEvent(),
 * updateDragPoint() should be called. The LSeat::toplevelMoveSessions() provides access to all active move sessions,
 * and the triggeringEvent() can be used to distinguish between pointer, touch or other kinds of sessions.
 *
 * To dynamically modify constraints, a callback function can be specified with setOnBeforeUpdateCallback(),
 * which is triggered each time before the toplevel position is updated. Within the callback function,
 * setConstraints() can be used to update the constraints().
 *
 * Finally, when the pointer button or touch point is released, the stop() method should be invoked to end the session
 * and remove it from the LSeat::toplevelMoveSessions() vector. The stop() method also returns an iterator pointing to the
 * next session in the vector which can be used in cases where the session is stopped while iterating through the vector.
 */
class Louvre::LToplevelMoveSession
{
public:

    /**
     *  @brief Callback function type used in setOnBeforeUpdateCallback()
     */
    using OnBeforeUpdateCallback = std::function<void(LToplevelMoveSession*)>;

    /**
     * @brief Start the move session.
     *
     * @note The session will automatically cease if the toplevel is destroyed.
     *
     * @param triggeringEvent The triggering event provided in LToplevelRole::startMoveRequest().
     * @param initDragPoint The initial pointer, touch, or other position at the start of the move operation.
     *
     * @returns `true` if the session successfully started, `false` if the toplevel is already in a session.
     */
    bool start(const LEvent &triggeringEvent, const LPoint &initDragPoint);

    /**
     * @brief Updates the drag point, causing the toplevel to move.
     *
     * @param pos The new position of the pointer or touch point.
     */
    void updateDragPoint(const LPoint &pos);

    /**
     * @brief Stops the move session.
     *
     * This method ends the current move session and removes it from the vector of active sessions.
     *
     * @return An iterator pointing to the next session within the LSeat::toplevelMoveSessions() vector.
     */
    const std::vector<LToplevelMoveSession*>::const_iterator stop();

    /**
     * @brief Sets the constraints for each side in compositor-global coordinates.
     *
     * @param constraints The margins specifying the constraints for each side. Use @ref LEdgeDisabled to disable constraints on specific sides.
     */
    void setConstraints(const LMargins &constraints = {LEdgeDisabled, LEdgeDisabled, LEdgeDisabled ,LEdgeDisabled}) noexcept
    {
        m_constraints = constraints;
    }

    /**
     * @brief Retrieves the constraints set with setConstraints().
     */
    const LMargins &constraints() const noexcept
    {
        return m_constraints;
    }

    /**
     * @brief Sets a custom callback function to be triggered each time before updating the toplevel position.
     *
     * @param callback The callback function to be executed before updating the toplevel window or `nullptr` to disable.
     *
     * #### Default Implementation
     * @snippet LToplevelMoveSession.cpp setCallback
     */
    void setOnBeforeUpdateCallback(const OnBeforeUpdateCallback &callback) noexcept
    {
        m_beforeUpdateCallback = callback;
    }

    /**
     * @brief Retrieves the LToplevelRole associated with this session.
     */
    LToplevelRole *toplevel() const noexcept
    {
        return m_toplevel;
    }

    /**
     * @brief Retrieves the triggering event provided during the start of the session.
     */
    const LEvent &triggeringEvent() const noexcept
    {
        return *m_triggeringEvent.get();
    }

    /**
     * @brief Checks if the session is currently active.
     *
     * @return `true` if the session is active, `false` otherwise.
     */
    bool isActive() const noexcept
    {
        return m_isActive;
    }

private:
    friend class LToplevelRole;
    LToplevelMoveSession(LToplevelRole *toplevel) noexcept;
    ~LToplevelMoveSession() noexcept;
    LToplevelRole *m_toplevel;
    LPoint m_initPos;
    LPoint m_initDragPoint;
    LMargins m_constraints {LEdgeDisabled, LEdgeDisabled, LEdgeDisabled ,LEdgeDisabled};
    std::unique_ptr<LEvent> m_triggeringEvent;
    OnBeforeUpdateCallback m_beforeUpdateCallback { nullptr };
    bool m_isActive { false };
    bool m_inCallback { false };
};

#endif // LTOPLEVELMOVESESSION_H
