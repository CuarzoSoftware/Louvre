#ifndef LTOPLEVELMOVESESSION_H
#define LTOPLEVELMOVESESSION_H

#include <LToplevelRole.h>

class Louvre::LToplevelMoveSession : public LObject
{
public:

    // TODO: Add doc

    /**
     * @brief Initiate an interactive toplevel moving session.
     *
     * This method initiates an interactive moving session for a toplevel surface.\n
     * You can confine the Toplevel's placement within a rectangle by specifying values for L, T, R, and B.\n
     * If you don't wish to restrict any edges, set their values to LPointer::EdgeDisabled.
     *
     * To update the Toplevel's position, use the updateMovingToplevelPos() method.
     * Once the position change is complete, use the stopMovingToplevel() method to conclude the session.
     *
     * @note The session will automatically cease if the toplevel is destroyed.
     *
     * @see See an example of its use in LToplevelRole::startMoveRequest().
     *
     * @param globalDragPoint Current move point position (cursor position, touch point position, etc). TODO
     * @param L Restriction for the left edge.
     * @param T Restriction for the top edge.
     * @param R Restriction for the right edge.
     * @param B Restriction for the bottom edge.
     */
    bool start(const LEvent &triggeringEvent,
               const LPoint &initDragPoint,
               Int32 L = LToplevelRole::EdgeDisabled, Int32 T = LToplevelRole::EdgeDisabled,
               Int32 R = LToplevelRole::EdgeDisabled, Int32 B = LToplevelRole::EdgeDisabled);

    void updateDragPoint(const LPoint &pos);

    const std::vector<LToplevelMoveSession*>::const_iterator stop();

    inline LToplevelRole *toplevel() const
    {
        return m_toplevel;
    }

    inline const LEvent &triggeringEvent() const
    {
        return *m_triggeringEvent.get();
    }

    inline bool isActive() const
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
    LBox m_bounds;
    std::unique_ptr<LEvent> m_triggeringEvent;
    bool m_isActive { false };
};

#endif // LTOPLEVELMOVESESSION_H
