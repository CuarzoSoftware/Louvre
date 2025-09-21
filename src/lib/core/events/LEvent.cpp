#include <LEvent.h>
#include <LKeyboardEnterEvent.h>
#include <LKeyboardKeyEvent.h>
#include <LKeyboardLeaveEvent.h>
#include <LKeyboardModifiersEvent.h>
#include <LPointerButtonEvent.h>
#include <LPointerEnterEvent.h>
#include <LPointerHoldBeginEvent.h>
#include <LPointerHoldEndEvent.h>
#include <LPointerLeaveEvent.h>
#include <LPointerMoveEvent.h>
#include <LPointerPinchBeginEvent.h>
#include <LPointerPinchEndEvent.h>
#include <LPointerPinchUpdateEvent.h>
#include <LPointerScrollEvent.h>
#include <LPointerSwipeBeginEvent.h>
#include <LPointerSwipeEndEvent.h>
#include <LPointerSwipeUpdateEvent.h>
#include <LTouchCancelEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>

using namespace Louvre;

LEvent *LEvent::copy() const noexcept {
  switch (m_type) {
    case Type::Pointer: {
      switch (m_subtype) {
        case Subtype::Enter:
          return (LEvent *)new LPointerEnterEvent(
              (const LPointerEnterEvent &)*this);
        case Subtype::Leave:
          return (LEvent *)new LPointerLeaveEvent(
              (const LPointerLeaveEvent &)*this);
        case Subtype::Move:
          return (LEvent *)new LPointerMoveEvent(
              (const LPointerMoveEvent &)*this);
        case Subtype::Button:
          return (LEvent *)new LPointerButtonEvent(
              (const LPointerButtonEvent &)*this);
        case Subtype::Scroll:
          return (LEvent *)new LPointerScrollEvent(
              (const LPointerScrollEvent &)*this);
        case Subtype::SwipeBegin:
          return (LEvent *)new LPointerSwipeBeginEvent(
              (const LPointerSwipeBeginEvent &)*this);
        case Subtype::SwipeUpdate:
          return (LEvent *)new LPointerSwipeUpdateEvent(
              (const LPointerSwipeUpdateEvent &)*this);
        case Subtype::SwipeEnd:
          return (LEvent *)new LPointerSwipeEndEvent(
              (const LPointerSwipeEndEvent &)*this);
        case Subtype::PinchBegin:
          return (LEvent *)new LPointerPinchBeginEvent(
              (const LPointerPinchBeginEvent &)*this);
        case Subtype::PinchUpdate:
          return (LEvent *)new LPointerPinchUpdateEvent(
              (const LPointerPinchUpdateEvent &)*this);
        case Subtype::PinchEnd:
          return (LEvent *)new LPointerPinchEndEvent(
              (const LPointerPinchEndEvent &)*this);
        case Subtype::HoldBegin:
          return (LEvent *)new LPointerHoldBeginEvent(
              (const LPointerHoldBeginEvent &)*this);
        case Subtype::HoldEnd:
          return (LEvent *)new LPointerHoldEndEvent(
              (const LPointerHoldEndEvent &)*this);
        case Subtype::Cancel:
        case Subtype::Up:
        case Subtype::Down:
        case Subtype::Key:
        case Subtype::Modifiers:
        case Subtype::Frame: return nullptr;
      }
      break;
    }
    case Type::Keyboard: {
      switch (m_subtype) {
        case Subtype::Enter:
          return (LEvent *)new LKeyboardEnterEvent(
              (const LKeyboardEnterEvent &)*this);
        case Subtype::Leave:
          return (LEvent *)new LKeyboardLeaveEvent(
              (const LKeyboardLeaveEvent &)*this);
        case Subtype::Key:
          return (LEvent *)new LKeyboardKeyEvent(
              (const LKeyboardKeyEvent &)*this);
        case Subtype::Modifiers:
          return (LEvent *)new LKeyboardModifiersEvent(
              (const LKeyboardModifiersEvent &)*this);
        case Subtype::Up:
        case Subtype::Down:
        case Subtype::Move:
        case Subtype::Button:
        case Subtype::Scroll:
        case Subtype::Frame:
        case Subtype::Cancel:
        case Subtype::SwipeBegin:
        case Subtype::SwipeUpdate:
        case Subtype::SwipeEnd:
        case Subtype::PinchBegin:
        case Subtype::PinchUpdate:
        case Subtype::PinchEnd:
        case Subtype::HoldBegin:
        case Subtype::HoldEnd: return nullptr;
      }
      break;
    }
    case Type::Touch: {
      switch (m_subtype) {
        case Subtype::Down:
          return (LEvent *)new LTouchDownEvent((const LTouchDownEvent &)*this);
        case Subtype::Move:
          return (LEvent *)new LTouchMoveEvent((const LTouchMoveEvent &)*this);
        case Subtype::Up:
          return (LEvent *)new LTouchUpEvent((const LTouchUpEvent &)*this);
        case Subtype::Frame:
          return (LEvent *)new LTouchFrameEvent(
              (const LTouchFrameEvent &)*this);
        case Subtype::Cancel:
          return (LEvent *)new LTouchCancelEvent(
              (const LTouchCancelEvent &)*this);
        case Subtype::Enter:
        case Subtype::Leave:
        case Subtype::Button:
        case Subtype::Key:
        case Subtype::Modifiers:
        case Subtype::Scroll:
        case Subtype::SwipeBegin:
        case Subtype::SwipeUpdate:
        case Subtype::SwipeEnd:
        case Subtype::PinchBegin:
        case Subtype::PinchUpdate:
        case Subtype::PinchEnd:
        case Subtype::HoldBegin:
        case Subtype::HoldEnd: return nullptr;
      }
      break;
    }
  }

  return nullptr;
}
