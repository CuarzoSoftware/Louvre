#include <LToplevelResizeSession.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LPointerEnterEvent.h>
#include <LSurface.h>

using namespace Louvre;

LToplevelResizeSession::LToplevelResizeSession() : m_triggeringEvent(std::make_unique<LPointerEnterEvent>()) {}

LToplevelResizeSession::~LToplevelResizeSession()
{
    if (m_isActive)
        LVectorRemoveOneUnordered(LCompositor::compositor()->seat()->imp()->resizeSessions, this);
}

void LToplevelResizeSession::handleGeometryChange()
{
    if (!m_toplevel->resizing())
        return;

    if (!m_isActive && m_lastSerial < m_toplevel->currentConfigureSerial())
    {
        m_lastSerial = 0;
        return;
    }

    if (m_edge == LToplevelRole::Top || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::TopRight)
        m_toplevel->surface()->setY(m_initPos.y() + (m_initSize.h() - m_toplevel->windowGeometry().h()));

    if (m_edge == LToplevelRole::Left || m_edge == LToplevelRole::TopLeft || m_edge == LToplevelRole::BottomLeft)
        m_toplevel->surface()->setX(m_initPos.x() + (m_initSize.w() - m_toplevel->windowGeometry().w()));

    if (!m_isActive)
        m_toplevel->configure(m_toplevel->pendingStates() & ~LToplevelRole::Resizing);
}

void LToplevelResizeSession::updateDragPoint(const LPoint &point)
{
    if (!m_isActive)
        return;

    m_currentDragPoint = point;
    LSize newSize = { toplevel()->calculateResizeSize(m_initDragPoint - point, m_initSize, m_edge) };

    const LPoint &pos { toplevel()->surface()->pos() };
    const LSize &size { toplevel()->size() };

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
    m_lastSerial = m_toplevel->pendingConfigureSerial();
}

bool LToplevelResizeSession::start(const LEvent &triggeringEvent, LToplevelRole::ResizeEdge edge, const LPoint &initDragPoint, const LSize &minSize, Int32 L, Int32 T, Int32 R, Int32 B)
{
    if (m_isActive)
        return false;

    m_isActive = true;
    m_triggeringEvent.reset(triggeringEvent.copy());
    m_minSize = minSize;
    m_bounds = {L,T,R,B};
    m_edge = edge;
    m_initSize = m_toplevel->size();
    m_initDragPoint = initDragPoint;
    m_currentDragPoint = initDragPoint;

    if (L != LToplevelRole::EdgeDisabled && m_toplevel->surface()->pos().x() < L)
        m_toplevel->surface()->setX(L);

    if (T != LToplevelRole::EdgeDisabled && m_toplevel->surface()->pos().y() < T)
        m_toplevel->surface()->setY(T);

    m_initPos = m_toplevel->surface()->pos();

    m_toplevel->configure(m_toplevel->pendingStates() | LToplevelRole::Activated | LToplevelRole::Resizing);
    m_lastSerial = m_toplevel->pendingConfigureSerial();
    LCompositor::compositor()->seat()->imp()->resizeSessions.push_back(this);
    return true;
}

const std::vector<LToplevelResizeSession*>::const_iterator LToplevelResizeSession::stop()
{
    if (!m_isActive)
        return LCompositor::compositor()->seat()->imp()->resizeSessions.begin();

    m_isActive = false;
    toplevel()->configure(toplevel()->pendingStates() & ~LToplevelRole::Resizing);
    auto it = std::find(LCompositor::compositor()->seat()->imp()->resizeSessions.begin(), LCompositor::compositor()->seat()->imp()->resizeSessions.end(), this);
    return LCompositor::compositor()->seat()->imp()->resizeSessions.erase(it);
}
