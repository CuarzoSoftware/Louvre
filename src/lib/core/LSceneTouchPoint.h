#ifndef LSCENETOUCHPOINT_H
#define LSCENETOUCHPOINT_H

#include <LObject.h>
#include <LPoint.h>

class Louvre::LSceneTouchPoint : public LObject
{
public:

    // TODO DISABLE COPY

    inline LScene *scene() const noexcept
    {
        return m_scene;
    }

    /**
     * @brief Get the unique identifier of the touch point.
     *
     * Each touch point is assigned a unique identifier, ensuring that there are no two touch points with the same ID.
     *
     * @return The unique identifier of the touch point.
     */
    inline Int32 id() const noexcept
    {
        return m_id;
    }

    /**
     * @brief Check if the touch point is currently being pressed.
     * @return True if the touch point is currently pressed, false otherwise.
     */
    inline bool pressed() const noexcept
    {
        return m_pressed;
    }

    /**
     * @brief Touched views
     *
     * Touched by this touch point. Updated each time a touch down event is handled.
     */
    inline const std::vector<LView*> &views() const noexcept
    {
        return m_views;
    }

    /**
     * @brief Get the position of the touch point assigned by the last touch-down or move event.
     *
     * The position is represented in a coordinate space ranging from 0 to 1 for both the x and y axes.
     *
     * @return A constant reference to the position of the touch point.
     */
    inline const LPointF &pos() const noexcept
    {
        return m_pos;
    }

private:
    friend class LScene;
    friend class LView;
    LSceneTouchPoint(LScene *scene, const LTouchDownEvent &event);
    inline ~LSceneTouchPoint() noexcept = default;
    std::vector<LSceneTouchPoint*>::iterator destroy();
    std::vector<LView*> m_views;
    LScene *m_scene { nullptr };
    LPointF m_pos;
    Int32 m_id { 0 };
    bool m_pressed { true };
    bool m_listChanged { false };
};


#endif // LSCENETOUCHPOINT_H
