#ifndef DOCK_H
#define DOCK_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include <LAnimation.h>

#include "Global.h"

using namespace Louvre;

class Output;

class Dock : public LLayerView
{
public:
    Dock(Output *output);
    ~Dock();

    void update();
    void show();
    void hide();

    // Dock textures
    LTextureView *dockLeft, *dockCenter, *dockRight;

    // Dock textures container
    LLayerView *dockContainer;

    // Container for minimized windows
    LLayerView *itemsContainer;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerMoveEvent(const LPoint &localPos) override;
    void pointerLeaveEvent() override;

    // Output of this dock
    Output *output = nullptr;

    // 0.f = HIDDEN, 1.0f = VISIBLE
    Float32 visiblePercent = 0.f;

    // HIDE/SHOW animation
    LAnimation *anim = nullptr;

    // Number of pointerMoveEvent() calls before show() is called
    UInt32 showResistance = 20;
    UInt32 showResistanceCount = 0;
};

#endif // DOCK_H
