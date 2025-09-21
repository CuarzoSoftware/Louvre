#include <LTouchDownEvent.h>
#include <LTouchEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>

using namespace Louvre;

Int32 LTouchEvent::id() const noexcept {
  if (subtype() == LEvent::Subtype::Down)
    return static_cast<const LTouchDownEvent*>(this)->m_id;
  else if (subtype() == LEvent::Subtype::Move)
    return static_cast<const LTouchMoveEvent*>(this)->m_id;
  else if (subtype() == LEvent::Subtype::Up)
    return static_cast<const LTouchUpEvent*>(this)->m_id;

  return -1;
}
