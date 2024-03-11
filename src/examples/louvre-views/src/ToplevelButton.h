#ifndef TOPLEVELBUTTON_H
#define TOPLEVELBUTTON_H

#include <LTextureView.h>
#include "UITextureView.h"

class ToplevelView;

class ToplevelButton final : public UITextureView
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
    bool pressed { false };

    void pointerButtonEvent(const LPointerButtonEvent &event) override;
    void pointerLeaveEvent(const LPointerLeaveEvent &) override;
    void pointerMoveEvent(const LPointerMoveEvent &) override;
};

#endif // TOPLEVELBUTTON_H
