#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Compositor;
class Surface;

class Pointer : public LPointer
{
public:
    Pointer(Params *params);
    Compositor *compositor() const;
    Surface *focus();
    LPoint viewLocalPos(LView *view);
    void pointerPosChangeEvent(Float32 x, Float32 y) override;
    void pointerButtonEvent(Button button, ButtonState state) override;
};

#endif // POINTER_H
