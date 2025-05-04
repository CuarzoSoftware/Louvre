#ifndef LTOPLEVELRESIZESESSION_H
#define LTOPLEVELRESIZESESSION_H

#include <LEdge.h>
#include <LMargins.h>
#include <LPoint.h>
#include <LTimer.h>
#include <memory>

/**
 * @brief Toplevel resize session utility
 *
 * This class serves as a utility for handling interactive resize sessions for LToplevelRole surfaces.\n
 * Each LToplevelRole has an associated resize session instance LToplevelRole::resizeSession().
 *
 * When a client triggers LToplevelRole::startResizeRequest(), the start() method should be invoked,
 * providing the triggering event and edges of the request and an initial drag point, which could be the current position of the pointer
 * (typically obtained via LCursor::pos()) or a touch point if triggered by a touch event. This also adds the session to the
 * LSeat::toplevelResizeSessions() vector.
 *
 * Subsequently, whenever the drag point changes, such as within an LPointer::pointerMoveEvent() or LTouch::touchMoveEvent(),
 * updateDragPoint() should be called. The LSeat::toplevelResizeSessions() provides access to all active resize sessions,
 * and the triggeringEvent() can be used to distinguish between pointer, touch or other kinds of sessions.
 *
 * To dynamically modify constraints, a callback function can be specified with setOnBeforeUpdateCallback(),
 * which is triggered each time before the toplevel position or size is updated. Within the callback function,
 * setConstraints() can be used to update the constraints().
 *
 * Finally, when the pointer button or touch point is released, the stop() method should be invoked to end the session
 * and remove it from the LSeat::toplevelResizeSessions() vector. The stop() method also returns an iterator pointing to the
 * next session in the vector which can be used in cases where the session is stopped while iterating through the vector.
 */
class Louvre::LToplevelResizeSession
{
public:

    /**
     *  @brief Callback function type used in setOnBeforeUpdateCallback()
     */
    using OnBeforeUpdateCallback = std::function<void(LToplevelResizeSession*)>;

    /**
     * @brief Start the resizing session.
     *
     * @note The session will automatically cease if the toplevel is destroyed.
     *
     * @param triggeringEvent The triggering event provided in LToplevelRole::startResizeRequest().
     * @param edge The edge or corner provided in LToplevelRole::startResizeRequest() indicating the starting edge of the resize operation.
     * @param initDragPoint The initial pointer, touch, or other position at the start of the resizing operation.
     *
     * @returns `true` if the session successfully started, `false` if the toplevel is already in a session.
     */
    bool start(const LEvent &triggeringEvent,
               LBitset<LEdge> edge,
               const LPoint &initDragPoint);

    /**
     * @brief Updates the drag point, causing the toplevel to move and/or resize.
     *
     * @param pos The new position of the pointer or touch point.
     */
    void updateDragPoint(const LPoint &pos);

    /**
     * @brief Stops the resizing session.
     *
     * This method ends the current resizing session and removes it from the vector of active sessions.
     *
     * @return An iterator pointing to the next session within the LSeat::toplevelResizeSessions() vector.
     */
    const std::vector<LToplevelResizeSession*>::const_iterator stop();

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
     * @brief Sets a minimum size for the toplevel.
     *
     * @param minSize The minimum size that the toplevel window can be resized to.
     */
    void setMinSize(const LSize &minSize) noexcept
    {
        m_minSize = minSize;
    }

    /**
     * @brief Retrieves the minimum size set with setMinSize().
     */
    const LSize &minSize() const noexcept
    {
        return m_minSize;
    }

    /**
     * @brief Sets a custom callback function to be triggered each time before updating the toplevel position or size.
     *
     * @param callback The callback function to be executed before updating the toplevel window or `nullptr` to disable.
     *
     * #### Default Implementation
     * @snippet LToplevelResizeSession.cpp setCallback
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
    LToplevelResizeSession(LToplevelRole *toplevel) noexcept;
    ~LToplevelResizeSession() noexcept;
    void handleGeometryChange();
    static constexpr LSize calculateResizeSize(const LPoint &cursorPosDelta, const LSize &initialSize, LBitset<LEdge> edge) noexcept
    {
        LSize newSize { initialSize };

        if (edge.check(LEdgeTop))
            newSize.setH(initialSize.h() + cursorPosDelta.y());
        else if(edge.check(LEdgeBottom))
            newSize.setH(initialSize.h() - cursorPosDelta.y());

        if (edge.check(LEdgeLeft))
            newSize.setW(initialSize.w() + cursorPosDelta.x());
        else if(edge.check(LEdgeRight))
            newSize.setW(initialSize.w() - cursorPosDelta.x());

        return newSize;
    }
    LToplevelRole *m_toplevel;
    LPoint m_initPos;
    LSize m_initSize;
    LSize m_minSize;
    LPoint m_initDragPoint;
    LPoint m_currentDragPoint;
    LBitset<LEdge> m_edge;
    LMargins m_constraints {LEdgeDisabled, LEdgeDisabled, LEdgeDisabled ,LEdgeDisabled};
    std::unique_ptr<LEvent> m_triggeringEvent;
    UInt32 m_lastSerial { 0 };
    bool m_isActive { false };
    bool m_lastSerialHandled { true };
    LTimer m_ackTimer;
    OnBeforeUpdateCallback m_beforeUpdateCallback { nullptr };
};

#endif // LTOPLEVELRESIZESESSION_H
