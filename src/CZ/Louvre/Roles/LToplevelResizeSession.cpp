#include <CZ/Louvre/Roles/LToplevelResizeSession.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Events/LPointerEnterEvent.h>
#include <CZ/Louvre/LSurface.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LUtils.h>
#include <CZ/Louvre/LLog.h>

using namespace Louvre;

LToplevelResizeSession::LToplevelResizeSession(LToplevelRole *toplevel) noexcept :
    m_toplevel(toplevel),
    m_triggeringEvent(std::make_unique<LPointerEnterEvent>())
{
//! [setCallback]
setOnBeforeUpdateCallback([](LToplevelResizeSession *session)
{
    LMargins constraints { session->toplevel()->calculateConstraintsFromOutput(cursor()->output()) };
    session->setConstraints(constraints);
});
//! [setCallback]

    m_ackTimer.setCallback([this](auto)
    {
        m_lastSerialHandled = true;
    });
}

LToplevelResizeSession::~LToplevelResizeSession() noexcept
{
    if (m_isActive)
        LVectorRemoveOneUnordered(seat()->imp()->resizeSessions, this);
}

void LToplevelResizeSession::handleGeometryChange()
{
    if (!m_lastSerialHandled)
    {
        if (m_edge.has(LEdgeTop))
            m_toplevel->surface()->setY(m_initPos.y() + (m_initSize.height() - m_toplevel->windowGeometry().height()));

        if (m_edge.has(LEdgeLeft))
            m_toplevel->surface()->setX(m_initPos.x() + (m_initSize.width() - m_toplevel->windowGeometry().width()));

        if (!m_isActive && (!m_toplevel->resizing() || m_lastSerial < m_toplevel->serial()) )
        {
            m_ackTimer.cancel();
            m_lastSerialHandled = true;
        }
    }
}

void LToplevelResizeSession::updateDragPoint(SkIPoint point)
{
    if (!m_isActive)
        return;

    if (m_beforeUpdateCallback)
        m_beforeUpdateCallback(this);

    if (!m_isActive)
        return;

    m_currentDragPoint = point;
    SkISize newSize = { calculateResizeSize(m_initDragPoint - point, m_initSize, m_edge) };

    SkIPoint pos { toplevel()->surface()->pos() };
    const SkISize &size { toplevel()->windowGeometry().size() };

    // Top
    if (m_constraints.top != LEdgeDisabled && m_edge.has(LEdgeTop))
    {
        if (pos.y() - (newSize.height() - size.height()) < m_constraints.top)
            newSize.fHeight = pos.y() + size.height() - m_constraints.top;
    }
    // Bottom
    else if (m_constraints.bottom != LEdgeDisabled && m_edge.has(LEdgeBottom))
    {
        if (pos.y() + newSize.height() > m_constraints.bottom)
            newSize.fHeight = m_constraints.bottom - pos.y();
    }

    // Left
    if (m_constraints.left != LEdgeDisabled && m_edge.has(LEdgeLeft))
    {
        if (pos.x() - (newSize.width() - size.width()) < m_constraints.left)
            newSize.fWidth = pos.x() + size.width() - m_constraints.left;
    }
    // Right
    else if (m_constraints.right != LEdgeDisabled && m_edge.has(LEdgeRight))
    {
        if (pos.x() + newSize.width() > m_constraints.right)
            newSize.fWidth = m_constraints.right - pos.x();
    }

    if (newSize.width() < m_minSize.width())
        newSize.fWidth = m_minSize.width();

    if (newSize.height() < m_minSize.height())
        newSize.fHeight = m_minSize.height();

    toplevel()->configureSize(newSize);
    toplevel()->configureState(LToplevelRole::Activated | LToplevelRole::Resizing);
    m_lastSerial = m_toplevel->pendingConfiguration().serial;
}

bool LToplevelResizeSession::start(const LEvent &triggeringEvent, CZBitset<LEdge> edge, SkIPoint initDragPoint)
{
    if (m_isActive)
        return false;

    m_isActive = true;

    if (m_beforeUpdateCallback)
        m_beforeUpdateCallback(this);

    if (!m_isActive)
        return false;

    m_ackTimer.cancel();
    m_lastSerialHandled = false;
    m_triggeringEvent.reset(triggeringEvent.copy());
    m_edge = edge;
    m_initSize = m_toplevel->windowGeometry().size();
    m_initDragPoint = initDragPoint;
    m_currentDragPoint = initDragPoint;

    if (m_constraints.left != LEdgeDisabled && m_toplevel->surface()->pos().x() < m_constraints.left)
        m_toplevel->surface()->setX(m_constraints.left);

    if (m_constraints.top != LEdgeDisabled && m_toplevel->surface()->pos().y() < m_constraints.top)
        m_toplevel->surface()->setY(m_constraints.top);

    m_initPos = m_toplevel->surface()->pos();

    m_toplevel->configureState(m_toplevel->pendingConfiguration().state | LToplevelRole::Activated | LToplevelRole::Resizing);
    m_lastSerial = m_toplevel->pendingConfiguration().serial;
    seat()->imp()->resizeSessions.push_back(this);
    return true;
}

const std::vector<LToplevelResizeSession*>::const_iterator LToplevelResizeSession::stop()
{
    if (!m_isActive)
        return seat()->imp()->resizeSessions.begin();

    m_isActive = false;
    toplevel()->configureState(toplevel()->pendingConfiguration().state & ~LToplevelRole::Resizing);
    m_lastSerialHandled = toplevel()->pendingConfiguration().size == toplevel()->m_lastACKConfiguration.size;

    if (!m_lastSerialHandled)
        m_ackTimer.start(600);

    auto it = std::find(seat()->imp()->resizeSessions.begin(), seat()->imp()->resizeSessions.end(), this);
    return seat()->imp()->resizeSessions.erase(it);
}


