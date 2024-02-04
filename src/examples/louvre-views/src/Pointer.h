#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer : public LPointer
{
public:
    Pointer(void *params);
    void pointerMoveEvent(Float32 x, Float32 y, bool absolute) override;
    void pointerButtonEvent(Button button, ButtonState state) override;
    void pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, AxisSource source) override;

    void setCursorRequest(LCursorRole *cursorRole) override;

    LView *cursorOwner = nullptr;
};

#endif // POINTER_H
