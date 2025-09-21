#include <LCompositor.h>
#include <LCursor.h>
#include <LPointerEnterEvent.h>
#include <LToplevelMoveSession.h>
#include <LUtils.h>
#include <private/LSeatPrivate.h>

#include <algorithm>

using namespace Louvre;

LToplevelMoveSession::LToplevelMoveSession(LToplevelRole *toplevel) noexcept
    : m_toplevel(toplevel),
      m_triggeringEvent(std::make_unique<LPointerEnterEvent>()) {
  //! [setCallback]
  setOnBeforeUpdateCallback([](LToplevelMoveSession *session) {
    LMargins constraints{session->toplevel()->calculateConstraintsFromOutput(
        cursor()->output())};
    constraints.bottom = LEdgeDisabled;
    session->setConstraints(constraints);
  });
  //! [setCallback]
}

LToplevelMoveSession::~LToplevelMoveSession() noexcept {
  if (m_isActive)
    LVectorRemoveOneUnordered(compositor()->seat()->imp()->moveSessions, this);
}

bool LToplevelMoveSession::start(const LEvent &triggeringEvent,
                                 const LPoint &initDragPoint) {
  if (m_isActive) return false;

  m_initDragPoint = initDragPoint;
  m_triggeringEvent.reset(triggeringEvent.copy());
  m_initPos = m_toplevel->surface()->pos();
  m_isActive = true;
  compositor()->seat()->imp()->moveSessions.push_back(this);
  return true;
}

void LToplevelMoveSession::updateDragPoint(const LPoint &pos) {
  if (!m_isActive) return;

  if (m_beforeUpdateCallback) m_beforeUpdateCallback(this);

  if (!m_isActive) return;

  LPoint newPos{m_initPos - m_initDragPoint + pos};

  if (m_constraints.right != LEdgeDisabled &&
      newPos.x() + m_toplevel->windowGeometry().w() > m_constraints.right)
    newPos.setX(m_constraints.right - m_toplevel->windowGeometry().w());

  if (m_constraints.left != LEdgeDisabled && newPos.x() < m_constraints.left)
    newPos.setX(m_constraints.left);

  if (m_constraints.bottom != LEdgeDisabled &&
      newPos.y() + m_toplevel->windowGeometry().h() > m_constraints.bottom)
    newPos.setY(m_constraints.bottom - m_toplevel->windowGeometry().h());

  if (m_constraints.top != LEdgeDisabled && newPos.y() < m_constraints.top)
    newPos.setY(m_constraints.top);

  toplevel()->surface()->setPos(newPos);
}

const std::vector<LToplevelMoveSession *>::const_iterator
LToplevelMoveSession::stop() {
  if (!m_isActive) return seat()->imp()->moveSessions.end();

  m_isActive = false;
  auto it = std::find(seat()->imp()->moveSessions.begin(),
                      seat()->imp()->moveSessions.end(), this);
  return seat()->imp()->moveSessions.erase(it);
}
