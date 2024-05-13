#ifndef LSCENETOUCHPOINT_H
#define LSCENETOUCHPOINT_H

#include <LObject.h>
#include <LPoint.h>

/**
 * @brief Touch point managed within an LScene.
 */
class Louvre::LSceneTouchPoint final : public LObject
{
public:

    LCLASS_NO_COPY(LSceneTouchPoint)

    /**
     * @brief Scene this touch point belongs to.
     */
    LScene *scene() const noexcept
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
    Int32 id() const noexcept
    {
        return m_id;
    }

    /**
     * @brief Check if the touch point is currently being pressed.
     */
    bool pressed() const noexcept
    {
        return m_pressed;
    }

    /**
     * @brief Touched views
     *
     * Vector of views being touched by this touch point.
     */
    const std::vector<LView*> &views() const noexcept
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
    const LPointF &pos() const noexcept
    {
        return m_pos;
    }

private:
    friend class LScene;
    friend class LView;
    LSceneTouchPoint(LScene *scene, const LTouchDownEvent &event) noexcept;
    ~LSceneTouchPoint() noexcept = default;
    std::vector<LSceneTouchPoint*>::iterator destroy() noexcept;
    std::vector<LView*> m_views;
    LScene *m_scene { nullptr };
    LPointF m_pos;
    Int32 m_id { 0 };
    bool m_pressed { true };
    bool m_listChanged { false };
};


#endif // LSCENETOUCHPOINT_H
