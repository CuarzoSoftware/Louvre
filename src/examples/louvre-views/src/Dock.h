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

    LLayerView *dockContainer;
    LTextureView *dockLeft, *dockCenter, *dockRight;
    LLayerView *itemsContainer;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerMoveEvent(const LPoint &localPos) override;
    void pointerLeaveEvent() override;

    Output *output = nullptr;
    Float32 visiblePercent = 0.f;
    LAnimation *anim = nullptr;

    UInt32 showResistance = 20;
    UInt32 showResistanceCount = 0;
};

#endif // DOCK_H
