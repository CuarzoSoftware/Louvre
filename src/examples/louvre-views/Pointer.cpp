#include <LScene.h>
#include <LCursor.h>
#include <LXCursor.h>

#include "Global.h"
#include "Pointer.h"

Pointer::Pointer(Params *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(Float32 dx, Float32 dy)
{
    G::scene()->handlePointerMoveEvent(dx, dy);
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    G::scene()->handlePointerPosChangeEvent(x, y);
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    G::scene()->handlePointerButtonEvent(button, state);
}

void Pointer::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    G::scene()->handlePointerAxisEvent(-axisX, -axisY, -discreteX, -discreteY, source);
}
