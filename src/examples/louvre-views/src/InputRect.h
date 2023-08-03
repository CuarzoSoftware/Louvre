#ifndef INPUTRECT_H
#define INPUTRECT_H

#include <LLayerView.h>

class InputRect : public LLayerView
{
public:
    InputRect(LView *parent = nullptr, void *userData = nullptr, UInt32 id = 0);

    UInt32 id;
    void *userData;
    void (*onPointerEnter)(InputRect *, void *, const LPoint &localPos) = nullptr;
    void (*onPointerLeave)(InputRect *, void *) = nullptr;
    void (*onPointerMove)(InputRect *, void *, const LPoint &localPos) = nullptr;
    void (*onPointerButton)(InputRect *, void *, LPointer::Button button, LPointer::ButtonState state) = nullptr;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerLeaveEvent() override;
    void pointerMoveEvent(const LPoint &localPos) override;
    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;
};

#endif // INPUTRECT_H
