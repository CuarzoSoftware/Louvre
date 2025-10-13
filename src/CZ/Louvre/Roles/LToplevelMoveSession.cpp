#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <algorithm>

using namespace CZ;

LToplevelMoveSession::LToplevelMoveSession(LToplevelRole *toplevel) noexcept :
    m_toplevel(toplevel),
    m_triggeringEvent(std::make_unique<CZPointerEnterEvent>())
{
//! [setCallback]
setOnBeforeUpdateCallback([](LToplevelMoveSession *session)
{
    LMargins constraints { session->toplevel()->calculateConstraintsFromOutput(cursor()->output()) };
    constraints.bottom = CZEdgeDisabled;
    session->setConstraints(constraints);
});
//! [setCallback]
}

LToplevelMoveSession::~LToplevelMoveSession() noexcept
{
    if (m_isActive)
        CZVectorUtils::RemoveOneUnordered(compositor()->seat()->imp()->moveSessions, this);
}

bool LToplevelMoveSession::start(const CZEvent &triggeringEvent, SkIPoint initDragPoint)
{
    if (m_isActive)
        return false;

    m_initDragPoint = initDragPoint;
    m_triggeringEvent = triggeringEvent.copy();
    m_initPos = m_toplevel->surface()->pos();
    m_isActive = true;
    compositor()->seat()->imp()->moveSessions.push_back(this);
    return true;
}

void LToplevelMoveSession::updateDragPoint(SkIPoint pos)
{
    if (!m_isActive)
        return;

    if (m_beforeUpdateCallback)
        m_beforeUpdateCallback(this);

    if (!m_isActive)
        return;

    SkIPoint newPos { m_initPos - m_initDragPoint + pos };

    if (m_constraints.right != CZEdgeDisabled && newPos.x() + m_toplevel->windowGeometry().width() > m_constraints.right)
        newPos.fX = m_constraints.right - m_toplevel->windowGeometry().width();

    if (m_constraints.left != CZEdgeDisabled && newPos.x() < m_constraints.left)
        newPos.fX = m_constraints.left;

    if (m_constraints.bottom != CZEdgeDisabled && newPos.y() + m_toplevel->windowGeometry().height() > m_constraints.bottom)
        newPos.fY = m_constraints.bottom - m_toplevel->windowGeometry().height();

    if (m_constraints.top != CZEdgeDisabled && newPos.y() < m_constraints.top)
        newPos.fY = m_constraints.top;

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
