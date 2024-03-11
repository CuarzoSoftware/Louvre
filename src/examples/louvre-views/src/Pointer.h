#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer final : public LPointer
{
public:
    Pointer(const void *params);
    void pointerMoveEvent(const LPointerMoveEvent &event) override;
    void pointerButtonEvent(const LPointerButtonEvent &event) override;
    void pointerScrollEvent(const LPointerScrollEvent &event) override;
    LView *cursorOwner { nullptr };
};

#endif // POINTER_H
