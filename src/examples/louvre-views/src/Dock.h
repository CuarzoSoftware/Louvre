#ifndef DOCK_H
#define DOCK_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include <LAnimation.h>
#include "UITextureView.h"

using namespace Louvre;

class Output;

class Dock final : public LLayerView
{
public:
    Dock(Output *output);
    ~Dock();

    void update();
    void show();
    void hide();

    // Dock textures container
    LLayerView dockContainer;

    // Dock textures
    UITextureView dockLeft, dockCenter, dockRight;

    // Container for apps and minimized windows
    LLayerView appsContainer;
    LSolidColorView separator;
    LLayerView itemsContainer;

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerMoveEvent(const LPointerMoveEvent &) override;
    void pointerLeaveEvent(const LPointerLeaveEvent &) override;

    // Output of this dock
    Output *output = nullptr;

    // 0.f = HIDDEN, 1.0f = VISIBLE
    Float32 visiblePercent = 0.f;

    // HIDE/SHOW animation
    LAnimation anim;

    // Number of pointerMoveEvent() calls before show() is called
    UInt32 showResistance = 6;
    UInt32 showResistanceCount = 0;

    bool alive = true;
};

#endif // DOCK_H
