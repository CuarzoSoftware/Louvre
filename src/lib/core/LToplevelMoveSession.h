#ifndef LTOPLEVELMOVESESSION_H
#define LTOPLEVELMOVESESSION_H

#include <LObject.h>
#include <LPoint.h>

class Louvre::LToplevelMoveSession : public LObject
{
public:
    // TODO
    void updateDragPoint(const LPoint &pos);
    const std::vector<LToplevelMoveSession*>::const_iterator stop();

    inline LToplevelRole *toplevel() const
    {
        return m_toplevel.lock().get();
    }

    inline const LEvent &triggeringEvent() const
    {
        return *m_triggeringEvent.get();
    }

    class Factory;
private:
    friend class LToplevelMoveSession::Factory;
    LToplevelMoveSession(LToplevelRole *toplevel, const LEvent &triggeringEvent, const LPoint &initDragPos, const LBox &bounds);
    ~LToplevelMoveSession();
    LPoint m_initPos;
    LPoint m_initDragPos;
    LBox m_bounds;
    std::weak_ptr<LToplevelRole> m_toplevel;
    std::unique_ptr<LEvent> m_triggeringEvent;
    bool stopped { false };
};

#endif // LTOPLEVELMOVESESSION_H
