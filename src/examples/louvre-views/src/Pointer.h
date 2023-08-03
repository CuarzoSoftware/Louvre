#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer : public LPointer
{
public:
    Pointer(Params *params);
    void pointerMoveEvent(Float32 dx, Float32 dy) override;
    void pointerPosChangeEvent(Float32 x, Float32 y) override;
    void pointerButtonEvent(Button button, ButtonState state) override;
    void pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source) override;

    void setCursorRequest(LCursorRole *cursorRole) override;

    LView *cursorOwner = nullptr;
    LSurface *lastCursorRequestFocusedSurface = nullptr;
};

#endif // POINTER_H
