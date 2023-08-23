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

    void update();

    Output *output;

    // White bar
    LSolidColorView *background;

    // Louvre logo
    LTextureView *logo = nullptr;

    // Clock text
    LTextureView *clock = nullptr;

    // Current app title
    LTextureView *appName = nullptr;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerMoveEvent(const LPoint &) override;
};

#endif // TOPBAR_H
