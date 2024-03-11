#ifndef LSCENETOUCHPOINT_H
#define LSCENETOUCHPOINT_H

#include <LObject.h>
#include <LPoint.h>

class Louvre::LSceneTouchPoint : public LObject
{
public:

    // TODO DISABLE COPY

    LScene *scene() const;

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
     * @return True if the touch point is currently pressed, false otherwise.
     */
    bool isPressed() const;

    /**
     * @brief Touched views
     *
     * Touched by this touch point. Updated each time a touch down event is handled.
     */
    const std::vector<LView*> &views() const;

    /**
     * @brief Get the position of the touch point assigned by the last touch-down or move event.
     *
     * The position is represented in a coordinate space ranging from 0 to 1 for both the x and y axes.
     *
     * @return A constant reference to the position of the touch point.
     */
    const LPointF &pos() const;

    LPRIVATE_IMP_UNIQUE(LSceneTouchPoint)
    friend class LScene;
    LSceneTouchPoint(LScene *scene, const LTouchDownEvent &event);
    ~LSceneTouchPoint();
    std::vector<LSceneTouchPoint*>::iterator destroy();
};


#endif // LSCENETOUCHPOINT_H
