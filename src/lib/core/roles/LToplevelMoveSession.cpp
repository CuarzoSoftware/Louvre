#include <LToplevelMoveSession.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LPointerEnterEvent.h>
#include <algorithm>
#include <LUtils.h>

using namespace Louvre;

LToplevelMoveSession::LToplevelMoveSession() : m_triggeringEvent(std::make_unique<LPointerEnterEvent>()) {}

LToplevelMoveSession::~LToplevelMoveSession()
{
    if (m_isActive)
        LVectorRemoveOneUnordered(compositor()->seat()->imp()->moveSessions, this);
}

bool LToplevelMoveSession::start(const LEvent &triggeringEvent, const LPoint &initDragPoint, const LMargin &constraints)
{
    if (m_isActive)
        return false;

    m_bounds = (const LBox&)constraints;
    m_initDragPoint = initDragPoint;
    m_triggeringEvent.reset(triggeringEvent.copy());
    m_initPos = m_toplevel->surface()->pos();
    m_isActive = true;
    compositor()->seat()->imp()->moveSessions.push_back(this);
    return true;
}

void LToplevelMoveSession::updateDragPoint(const LPoint &pos)
{
    if (!m_isActive)
        return;

    LPoint newPos { m_initPos - m_initDragPoint + pos };

    if (m_bounds.x2 != LToplevelRole::EdgeDisabled && newPos.x() + m_toplevel->windowGeometry().w() > m_bounds.x2)
        newPos.setX(m_bounds.x2 - m_toplevel->windowGeometry().w());

    if (m_bounds.x1 != LToplevelRole::EdgeDisabled && newPos.x() < m_bounds.x1)
        newPos.setX(m_bounds.x1);

    if (m_bounds.y2 != LToplevelRole::EdgeDisabled && newPos.y() + m_toplevel->windowGeometry().h() > m_bounds.y2)
        newPos.setY(m_bounds.y2 - m_toplevel->windowGeometry().h());

    if (m_bounds.y1 != LToplevelRole::EdgeDisabled && newPos.y() < m_bounds.y1)
        newPos.setY(m_bounds.y1);

    toplevel()->surface()->setPos(newPos);
}

const std::vector<LToplevelMoveSession*>::const_iterator LToplevelMoveSession::stop()
{
    if (!m_isActive)
        return seat()->imp()->moveSessions.end();

    m_isActive = false;
    auto it = std::find(seat()->imp()->moveSessions.begin(),
                        seat()->imp()->moveSessions.end(),
                        this);
    return seat()->imp()->moveSessions.erase(it);
}
