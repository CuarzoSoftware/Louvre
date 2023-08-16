#ifndef TOPBAR_H
#define TOPBAR_H

#include <LLayerView.h>
#include <LSolidColorView.h>

class Output;

class Topbar : public LLayerView
{
public:
    Topbar(Output *output);
    ~Topbar();

    void show();
    void hide();
    void update();

    Output *output;

    Float32 visiblePercent = 1.f;

    // White bar
    LSolidColorView *background;

    // Louvre logo
    LTextureView *logo = nullptr;

    // Clock text
    LTextureView *clock = nullptr;

    // Current app title
    LTextureView *appName = nullptr;

    // SHOW/HIDE animation
    LAnimation *anim = nullptr;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerMoveEvent(const LPoint &) override;
    void pointerLeaveEvent() override;
};

#endif // TOPBAR_H
