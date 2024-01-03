#ifndef TOPLEVELBUTTON_H
#define TOPLEVELBUTTON_H

#include <LTextureView.h>
#include "UITextureView.h"

class ToplevelView;

class ToplevelButton : public UITextureView
{
public:
    enum ButtonType
    {
        Close,
        Minimize,
        Maximize
    };

    ToplevelButton(LView *parent, ToplevelView *toplevelView, ButtonType type);

    void update();

    ToplevelView *toplevelView;
    ButtonType buttonType;
    bool pressed = false;

    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;
    void pointerLeaveEvent() override;
    void pointerMoveEvent(const LPoint &) override;
};

#endif // TOPLEVELBUTTON_H
