#include <LToplevelMoveSession.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LEvent.h>
#include <algorithm>

using namespace Louvre;

LToplevelMoveSession::LToplevelMoveSession(LToplevelRole *toplevel, const LEvent &triggeringEvent, const LPoint &initDragPos, const LBox &bounds) :
    m_initPos(toplevel->surface()->pos()),
    m_initDragPos(initDragPos),
    m_bounds(bounds),
    m_toplevel(toplevel->weakRef<LToplevelRole>())
{
    m_triggeringEvent.reset(triggeringEvent.copy());
    seat()->imp()->moveSessions.push_back(this);
}

LToplevelMoveSession::~LToplevelMoveSession()
{
    if (!stopped)
    {
        auto it = std::find(seat()->imp()->moveSessions.begin(), seat()->imp()->moveSessions.end(), this);
        auto nextIt = seat()->imp()->moveSessions.end();

        if (it != seat()->imp()->moveSessions.end())
            nextIt = seat()->imp()->moveSessions.erase(it);
    }
}

void LToplevelMoveSession::updateDragPoint(const LPoint &pos)
{
    LPoint newPos { m_initPos - m_initDragPos + pos };

    if (m_bounds.x2 != LToplevelRole::EdgeDisabled && newPos.x() > m_bounds.x2)
        newPos.setX(m_bounds.x2);

    if (m_bounds.x1 != LToplevelRole::EdgeDisabled && newPos.x() < m_bounds.x1)
        newPos.setX(m_bounds.x1);

    if (m_bounds.y2 != LToplevelRole::EdgeDisabled && newPos.y() > m_bounds.y2)
        newPos.setY(m_bounds.y2);

    if (m_bounds.y1 != LToplevelRole::EdgeDisabled && newPos.y() < m_bounds.y1)
        newPos.setY(m_bounds.y1);

    toplevel()->surface()->setPos(newPos);
}

const std::vector<LToplevelMoveSession*>::const_iterator LToplevelMoveSession::stop()
{
    auto it = std::find(seat()->imp()->moveSessions.begin(), seat()->imp()->moveSessions.end(), this);
    auto nextIt = seat()->imp()->moveSessions.end();

    if (it != seat()->imp()->moveSessions.end())
        nextIt = seat()->imp()->moveSessions.erase(it);

    stopped = true;
    toplevel()->imp()->moveSession.reset();
    return nextIt;
}
