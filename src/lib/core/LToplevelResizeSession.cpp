#include <LToplevelResizeSession.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LSurface.h>
#include <LEvent.h>

using namespace Louvre;

LToplevelResizeSession::LToplevelResizeSession() {}

LToplevelResizeSession::~LToplevelResizeSession()
{
    delete m_triggeringEvent;
}

void LToplevelResizeSession::handleGeometryChange()
{
    if (!m_toplevel->resizing() && m_stopped)
    {
        /*TODO destroy(); */
        return;
    }

    if (!m_toplevel->resizing())
        return;

    if (m_edge ==  LToplevelRole::Top || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::TopRight)
        m_toplevel->surface()->setY(m_initPos.y() + (m_initSize.h() - m_toplevel->windowGeometry().h()));

    if (m_edge ==  LToplevelRole::Left || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::BottomLeft)
        m_toplevel->surface()->setX(m_initPos.x() + (m_initSize.w() - m_toplevel->windowGeometry().w()));

    if (m_stopped)
        m_toplevel->configure(m_toplevel->pendingStates() & ~LToplevelRole::Resizing);
}

void LToplevelResizeSession::setResizePointPos(const LPoint &resizePoint)
{
    if (m_stopped)
        return;

    m_currentResizePointPos = resizePoint;
    LSize newSize = toplevel()->calculateResizeSize(m_initResizePointPos - resizePoint,
                                                    m_initSize,
                                                    m_edge);
    LPoint pos = toplevel()->surface()->pos();
    LSize size = toplevel()->windowGeometry().size();

    // Top
    if (m_bounds.y1 != LToplevelRole::EdgeDisabled && (m_edge == LToplevelRole::Top || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::TopRight))
    {
        if (pos.y() - (newSize.y() - size.y()) < m_bounds.y1)
            newSize.setH(pos.y() + size.h() - m_bounds.y1);
    }
    // Bottom
    else if (m_bounds.y2 != LToplevelRole::EdgeDisabled && (m_edge == LToplevelRole::Bottom || m_edge == LToplevelRole::BottomLeft || m_edge == LToplevelRole::BottomRight))
    {
        if (pos.y() + newSize.h() > m_bounds.y2)
            newSize.setH(m_bounds.y2 - pos.y());
    }

    // Left
    if (m_bounds.x1 != LToplevelRole::EdgeDisabled && (m_edge == LToplevelRole::Left || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::BottomLeft))
    {
        if (pos.x() - (newSize.x() - size.x()) < m_bounds.x1)
            newSize.setW(pos.x() + size.w() - m_bounds.x1);
    }
    // Right
    else if (m_bounds.x2 != LToplevelRole::EdgeDisabled && (m_edge == LToplevelRole::Right || m_edge == LToplevelRole::TopRight || m_edge == LToplevelRole::BottomRight))
    {
        if (pos.x() + newSize.w() > m_bounds.x2)
            newSize.setW(m_bounds.x2 - pos.x());
    }

    if (newSize.w() < m_minSize.w())
        newSize.setW(m_minSize.w());

    if (newSize.h() < m_minSize.h())
        newSize.setH(m_minSize.h());

    toplevel()->configure(newSize, LToplevelRole::Activated | LToplevelRole::Resizing);
}

void LToplevelResizeSession::stop()
{
    /* TODO
    toplevel()->configure(toplevel()->pendingState() & ~LToplevelRole::Resizing);

    if (imp()->stopped)
        return seat()->imp()->resizeSessions.begin();

    std::list<LToplevelResizeSession*>::iterator it = imp()->unlink();
    imp()->stopped = true;

    return it;
    */
}

LToplevelRole *LToplevelResizeSession::toplevel() const
{
    return m_toplevel;
}

const LEvent &LToplevelResizeSession::triggeringEvent() const
{
    return *m_triggeringEvent;
}
