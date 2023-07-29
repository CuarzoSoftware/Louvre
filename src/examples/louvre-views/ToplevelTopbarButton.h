#ifndef TOPLEVELTOPBARBUTTON_H
#define TOPLEVELTOPBARBUTTON_H

#include <LSolidColorView.h>

class Toplevel;

class ToplevelTopbarButton : public LSolidColorView
{
public:

    enum TopBarButtonType
    {
        CloseButton,
        MinimizeButton,
        MaximizeButton
    };

    ToplevelTopbarButton(Toplevel *toplevel, TopBarButtonType type);

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerLeaveEvent() override;
    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;

    Toplevel *toplevel;
    TopBarButtonType type;
};

#endif // TOPLEVELTOPBARBUTTON_H
