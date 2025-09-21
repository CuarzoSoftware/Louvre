#include "InputRect.h"

#include <LPointer.h>
#include <LPointerEnterEvent.h>
#include <LPointerMoveEvent.h>

InputRect::InputRect(LView *parent, void *userData, UInt32 id)
    : LLayerView(parent) {
  setUserData((UIntPtr)parent);
  this->id = id;
  this->userData = userData;
  enableBlockPointer(true);
  enablePointerEvents(true);
}

void InputRect::pointerEnterEvent(const LPointerEnterEvent &event) {
  if (onPointerEnter) onPointerEnter(this, userData, event.localPos);
}

void InputRect::pointerLeaveEvent(const LPointerLeaveEvent &) {
  if (onPointerLeave) onPointerLeave(this, userData);
}

void InputRect::pointerMoveEvent(const LPointerMoveEvent &event) {
  if (onPointerMove) onPointerMove(this, userData, event.localPos);
}

void InputRect::pointerButtonEvent(const LPointerButtonEvent &event) {
  if (onPointerButton)
    onPointerButton(this, userData, event.button(), event.state());
}
