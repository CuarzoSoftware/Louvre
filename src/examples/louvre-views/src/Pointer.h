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
    void setCursorRequest(const LClientCursor &clientCursor) override;
    bool maybeMoveOrResize(const LPointerButtonEvent &event);

    LView *cursorOwner { nullptr };
    bool isDragging { false };
};

#endif // POINTER_H
