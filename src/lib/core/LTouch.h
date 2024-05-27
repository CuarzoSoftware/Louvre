#ifndef LTOUCH_H
#define LTOUCH_H

#include <LFactoryObject.h>

/**
 * @brief Class for handling touch input events
 *
 * The LTouch class facilitates the management of touch input events, allowing you to redirect them to client surfaces.\n
 * Touch events commence with a touch-down event, which can be utilized to create a new touch point using createOrGetTouchPoint().
 * Each touch point is assigned a unique ID obtained from the touch-down event. Invoking createOrGetTouchPoint() with an already
 * used ID returns the existing touch point associated with that ID. All touch points are initially marked as pressed and can
 * be accessed through the touchPoints() vector.
 *
 * Once a touch point is created, you can associate it with a surface using LTouchPoint::sendDownEvent(). Only a single surface
 * can be assigned to a touch point at a time, or none at all. Touch-down and move events received from the input backend contain
 * information about the current physical position of the touchpoint on the touch device, represented in a coordinate space
 * ranging from 0 to 1 for both x and y axes, with the top-left corner as the origin.
 *
 * There is no universal way to associate a touch device with an output. To achieve this, you could use information from the events
 * input device and transform touch point positions to output coordinates with the toGlobal() aux method.
 *
 * @note The default implementation transforms touchpoint coordinates to the output coordinates where the LCursor is currently positioned.
 *
 * After a touch point is created, subsequent touch-down, move, and up events may occur. These events can be forwarded to clients
 * using the methods provided by LTouchPoint. Clients are expected to wait for a frame event to process them atomically, which is triggered
 * by calling sendFrameEvent().
 *
 * The input backend can also notify of a cancel event, typically when a touch device is unplugged or an error occurs.\n
 * You can forward this event to clients using the sendCancelEvent() method. After invoking this method, all touch points are
 * destroyed, even if they haven't been released before with a touch-up event.
 */
class Louvre::LTouch : public LFactoryObject
{
public:

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LTouch;

    /**
     * @brief Constructor of the LTouch class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LTouch(const void *params) noexcept;

    LCLASS_NO_COPY(LTouch)

    /**
     * @brief Destructor of the LTouch class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LTouch() noexcept;

    /**
     * @brief Look for a surface.
     *
     * This method looks for the first mapped surface that contains the point given by the `point` parameter.\n
     * It takes into account the surfaces role position (LSurface::rolePos()), their input region (LSurface::inputRegion()) and the order
     * given by the list of surfaces of the compositor (LCompositor::surfaces()).\n
     * Some surface roles do not have an input region such as LCursorRole or LDNDIconRole so these surfaces are always ignored.
     *
     * @param point Point in compositor coordinates.
     * @returns Returns the first surface that contains the point or `nullptr` if no surface is found.
     */
    LSurface *surfaceAt(const LPoint &point) const noexcept;

    /**
     * @brief Vector of currently active touchpoints.
     *
     * Each touchpoint has a unique identifier and can be associated with a single surface at a time or none at all.
     *
     * @return A constant reference to the vector of currently active touchpoints.
     */
    const std::vector<LTouchPoint*> &touchPoints() const noexcept;

    /**
     * @brief Creates a new touch point or returns an existing one with the same id.
     *
     * This method generates a new touch point based on the provided touch down event id.\n
     * If a touch point with the same id already exists, that existing touchpoint is returned instead.\n
     * Newly created touchpoints are initially marked as pressed.
     *
     * @param event The touch down event used to create the touch point.
     * @return A pointer to the newly created or existing touch point.
     */
    LTouchPoint *createOrGetTouchPoint(const LTouchDownEvent &event) noexcept;

    /**
     * @brief Get the touch point that matches the specified id.
     *
     * This method searches for a touch point with the given id. If found, a pointer to the matching touch point is returned;
     * otherwise, `nullptr` is returned.
     *
     * @param id The unique identifier of the touch point to be found.
     * @return A pointer to the matching touch point or `nullptr` if no touch point with the id is found.
     */
    LTouchPoint *findTouchPoint(Int32 id) const noexcept;

    /**
     * @brief Transforms a touch point position to global coordinates.
     *
     * This method enables the conversion of a touch point position, defined within the range of 0 to 1
     * for both x and y axes, to a global position. The transformation takes into account the specified output's position,
     * size, and any applied transform.
     *
     * @return The transformed global position of the touch point, or the same position if output is `nullptr`.
     */
    static LPointF toGlobal(LOutput *output, const LPointF &touchPointPos) noexcept;

    /**
     * @brief Send a frame event to all clients with surfaces assigned to touch points.
     *
     * Clients are expected to wait for this event before processing previously sent touch events.\n
     * After this frame event, touch points that are no longer pressed are destroyed.
     *
     * @param event The frame event to be sent to clients.
     */
    void sendFrameEvent(const LTouchFrameEvent &event) noexcept;

    /**
     * @brief Send a cancel event to clients with surfaces assigned to touchpoints.
     *
     * This method notifies clients of a cancel event, and subsequently, all current touchpoints are destroyed.
     *
     * @param event The touch cancel event to be sent to clients.
     */
    void sendCancelEvent(const LTouchCancelEvent &event) noexcept;

    /**
     * @brief Triggered by the input backend when a new touch point is created.
     *
     * This virtual method is called when a touch-down event occurs, signaling the creation of a new touch point.
     *
     * @param event The touch-down event providing details about the new touch point.
     *
     * #### Default Implementation
     *
     * @snippet LTouchDefault.cpp touchDownEvent
     */
    virtual void touchDownEvent(const LTouchDownEvent &event);

    /**
     * @brief Triggered by the input backend when a pressed touchpoint moves.
     *
     * This virtual method is called when a touch-move event occurs, indicating the movement of a pressed touchpoint.
     *
     * @param event The touch-move event providing details about the moving touchpoint.
     *
     * #### Default Implementation
     *
     * @snippet LTouchDefault.cpp touchMoveEvent
     */
    virtual void touchMoveEvent(const LTouchMoveEvent &event);

    /**
     * @brief Triggered by the input backend when a touchpoint is no longer pressed.
     *
     * This virtual method is called when a touch-up event occurs, indicating that a touchpoint is no longer pressed.
     *
     * @param event The touch-up event providing details about the released touchpoint.
     *
     * #### Default Implementation
     *
     * @snippet LTouchDefault.cpp touchUpEvent
     */
    virtual void touchUpEvent(const LTouchUpEvent &event);

    /**
     * @brief Triggered by the input backend after sending down, move, and up events that logically belong together
     *        and should be processed atomically.
     *
     * This virtual method is called when a touch frame event occurs, signifying the completion of a set of related
     * touch events (down, move, up) that should be processed as a single atomic unit.
     *
     * @param event The touch frame event providing details about the synchronized touch events.
     *
     * #### Default Implementation
     *
     * @snippet LTouchDefault.cpp touchFrameEvent
     */
    virtual void touchFrameEvent(const LTouchFrameEvent &event);

    /**
     * @brief Triggered by the input backend when all active touchpoints are cancelled, typically
     *        in response to an unavailable touch input device.
     *
     * This virtual method is called when a touch cancel event occurs, indicating the cancellation of all active
     * touchpoints, often triggered by the unavailability of a touch input device.
     *
     * @param event The touch cancel event providing details about the cancellation.
     *
     * #### Default Implementation
     *
     * @snippet LTouchDefault.cpp touchCancelEvent
     */
    virtual void touchCancelEvent(const LTouchCancelEvent &event);

private:
    friend class LTouchPoint;
    mutable std::vector<LTouchPoint*> m_touchPoints;
};

#endif // LTOUCH_H
