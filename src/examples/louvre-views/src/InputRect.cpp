#include "InputRect.h"

InputRect::InputRect(LView *parent, void *userData, UInt32 id) : LLayerView(parent)
{
    this->id = id;
    this->userData = userData;
    enableBlockPointer(true);
    enableInput(true);
}

void InputRect::pointerEnterEvent(const LPoint &localPos)
{
    if (onPointerEnter)
        onPointerEnter(this, userData, localPos);
}

void InputRect::pointerLeaveEvent()
{
    if (onPointerLeave)
        onPointerLeave(this, userData);
}

void InputRect::pointerMoveEvent(const LPoint &localPos)
{
    if (onPointerMove)
        onPointerMove(this, userData, localPos);
}

void InputRect::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (onPointerButton)
        onPointerButton(this, userData, button, state);
}
