#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer : public LPointer
{
public:
    Pointer(Params *params);
    void pointerMoveEvent(Float32 x, Float32 y, bool absolute) override;
    void pointerButtonEvent(Button button, ButtonState state) override;
};

#endif // POINTER_H
