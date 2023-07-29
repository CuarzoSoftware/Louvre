#ifndef TOPLEVELTOPBAR_H
#define TOPLEVELTOPBAR_H

#include <LSolidColorView.h>

class Toplevel;
class ToplevelTopbarButton;

class ToplevelTopbar : public LSolidColorView
{
public:
    ToplevelTopbar(Toplevel *toplevel);
    ~ToplevelTopbar();

    void update();

    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;

    Toplevel *toplevel = nullptr;

    ToplevelTopbarButton *closeButton, *minimizeButton, *maximizeButton;

};

#endif // TOPLEVELTOPBAR_H
