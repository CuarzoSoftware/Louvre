#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer : public LPointer
{
public:
    Pointer(Params *params);
    float lastEventMs;
    void pointerMoveEvent(float dx, float dy) override;
    void pointerPosChangeEvent(Float32 x, Float32 y) override;
    void pointerButtonEvent(Button button, ButtonState state) override;
};

#endif // POINTER_H
