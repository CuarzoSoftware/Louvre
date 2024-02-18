#if 1 == 2
#ifndef LTOUCHPOINT_H
#define LTOUCHPOINT_H

#include <LObject.h>
#include <LPoint.h>

/**
 * @brief Touch point within a touch device.
 *
 * An instance of this class is created using LTouch::createTouchPoint() and is typically destroyed after calling
 * LTouch::sendFrameEvent() when the touch point is no longer pressed, or when invoking LTouch::sendCancelEvent().
 *
 * A touch point can be assigned to a single surface at a time or none at all. This assignment is performed using the
 * sendDownEvent() method.
 *
 * To gain a comprehensive understanding of touch events, please refer to the documentation of LTouch.
 *
 * @note When sending a touch-down or move event to a surface, ensure that the position is specified in local surface coordinates.
 *       Both LTouchDownEvent and LTouchMoveEvent contain a mutable variable named localPos. Be sure to set its value properly to
 *       the local surface coordinate before dispatching these events.
 */
class Louvre::LTouchPoint : public LObject
{
public:

    LCLASS_NO_COPY(LTouchPoint)

    /**
     * @brief Get the unique identifier of the touch point.
     *
     * Each touch point is assigned a unique identifier, ensuring that there are no two touch points with the same ID.
     *
     * @return The unique identifier of the touch point.
     */
    Int32 id() const;

    /**
     * @brief Check if the touch point is currently being pressed.
     *
     * This method indicates whether the touch point is in a pressed state. Initially, all touch points are marked as pressed
     * upon creation. The state can change if either the sendDownEvent() or sendUpEvent() methods are invoked.
     *
     * If LTouch::sendFrameEvent() is called and the touch point is no longer pressed, it will be automatically destroyed.
     *
     * @return True if the touch point is currently pressed, false otherwise.
     */
    bool isPressed() const;

    /**
     * @brief Get the surface currently being touched by this touch point.
     *
     * The surface associated with this touch point is set using the sendDownEvent() method. If no surface is assigned to this touch point,
     * the method returns nullptr.
     *
     * @return A pointer to the LSurface being touched by this touch point, or nullptr if no surface is assigned.
     */
    LSurface *surface() const;

    /**
     * @brief Get the last touch-down event sent with sendDownEvent().
     * @return The last touch-down event.
     */
    const LTouchDownEvent &lastDownEvent() const;

    /**
     * @brief Get the last touch move event sent with sendMoveEvent().
     * @return The last touch move event.
     */
    const LTouchMoveEvent &lastMoveEvent() const;

    /**
     * @brief Get the last touch-up event sent with sendUpEvent().
     * @return The last touch-up event.
     */
    const LTouchUpEvent &lastUpEvent() const;

    /**
     * @brief Get the position of the touch point assigned by the last touch-down or move event.
     *
     * The position is represented in a coordinate space ranging from 0 to 1 for both the x and y axes.
     *
     * @return A constant reference to the position of the touch point.
     */
    const LPointF &pos() const;

    /**
     * @brief Mark the touch point as pressed.
     *
     * If a surface is specified, the event is sent to that surface. If the surface is different from the current surface,
     * the previous surface receives a touch-up event and a frame event.
     * If the surface parameter is nullptr, it unsets the current surface.
     * If the current assigned surface is destroyed, it is automatically unset.
     *
     * @note The LTouchDownEvent::localPos mutable variable must be set to represent the position in local surface coordinates.
     *
     * @param event The touch-down event with information about the touch point
     * @param surface The surface to which the event should be sent. If nullptr, unsets the surface.
     * @return True on success, false if the IDs don't match.
     */
    bool sendDownEvent(const LTouchDownEvent &event, LSurface *surface = nullptr);

    /**
     * @brief Notify the client about the movement of the touch point if there is an assigned surface.
     *
     * @note The LTouchMoveEvent::localPos mutable variable must be set to represent the position in local surface coordinates.
     *
     * @param event The touch move event containing information about the touch point movement.
     * @return True on success, false if the IDs don't match.
     */
    bool sendMoveEvent(const LTouchMoveEvent &event);

    /**
     * @brief Notify the client about the touch point being released if there is an assigned surface.
     *
     * If LTouch::sendFrameEvent() is called and the touch point is no longer pressed, it is automatically destroyed.
     *
     * @param event The touch-up event containing information about the touch point release.
     * @return True on success, false if the IDs don't match.
     */
    bool sendUpEvent(const LTouchUpEvent &event);

LPRIVATE_IMP_UNIQUE(LTouchPoint)
    friend class LTouch;
    LTouchPoint(const LTouchDownEvent &event);
    ~LTouchPoint();
};

#endif // LTOUCHPOINT_H
#endif
