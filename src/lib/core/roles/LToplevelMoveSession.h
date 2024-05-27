#ifndef LTOPLEVELMOVESESSION_H
#define LTOPLEVELMOVESESSION_H

#include <LToplevelRole.h>

class Louvre::LToplevelMoveSession
{
public:

    // TODO: Add doc (and update)

    using OnBeforeUpdateCallback = std::function<void(LToplevelMoveSession*)>;

    /**
     * @brief Initiate an interactive toplevel moving session.
     *
     * This method initiates an interactive moving session for a toplevel surface.\n
     * You can confine the Toplevel's placement within a rectangle specified by the constraints argument.\n
     * Use LToplevelRole:EdgeDisabled to disable the constraint for a specific edge.
     *
     * To update the Toplevel's position, use the updateMovingToplevelPos() method.
     * Once the position change is complete, use the stopMovingToplevel() method to conclude the session.
     *
     * @note The session will automatically cease if the toplevel is destroyed.
     *
     * @see See an example of its use in LToplevelRole::startMoveRequest().
     *
     * @param globalDragPoint Current move point position (cursor position, touch point position, etc). TODO
     * @param L Restriction for each edge.
     * @param T Restriction for the top edge.
     * @param R Restriction for the right edge.
     * @param B Restriction for the bottom edge.
     */
    bool start(const LEvent &triggeringEvent, const LPoint &initDragPoint);

    void setConstraints(const LMargins &constraints = {LToplevelRole::EdgeDisabled, LToplevelRole::EdgeDisabled, LToplevelRole::EdgeDisabled ,LToplevelRole::EdgeDisabled}) noexcept
    {
        m_constraints = constraints;
    }

    const LMargins &constraints() const noexcept
    {
        return m_constraints;
    }

    void setOnBeforeUpdateCallback(const OnBeforeUpdateCallback &callback) noexcept
    {
        m_beforeUpdateCallback = callback;
    }

    void updateDragPoint(const LPoint &pos);

    const std::vector<LToplevelMoveSession*>::const_iterator stop();

    LToplevelRole *toplevel() const noexcept
    {
        return m_toplevel;
    }

    const LEvent &triggeringEvent() const noexcept
    {
        return *m_triggeringEvent.get();
    }

    bool isActive() const noexcept
    {
        return m_isActive;
    }

private:
    friend class LToplevelRole;
    LToplevelMoveSession();
    ~LToplevelMoveSession();
    LToplevelRole *m_toplevel;
    LPoint m_initPos;
    LPoint m_initDragPoint;
    LMargins m_constraints {LToplevelRole::EdgeDisabled, LToplevelRole::EdgeDisabled, LToplevelRole::EdgeDisabled ,LToplevelRole::EdgeDisabled};
    std::unique_ptr<LEvent> m_triggeringEvent;
    OnBeforeUpdateCallback m_beforeUpdateCallback { nullptr };
    bool m_isActive { false };
    bool m_inCallback { false };
};

#endif // LTOPLEVELMOVESESSION_H
