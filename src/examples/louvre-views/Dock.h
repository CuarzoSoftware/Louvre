#ifndef DOCK_H
#define DOCK_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include <LAnimation.h>
#include <Global.h>

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

    LLayerView dockClipping = LLayerView(this);
    LSceneView *dockScene;
    LSolidColorView *dockBackground;
    LLayerView *itemsContainer;

    LTextureView *borderRadiusTL;
    LTextureView *borderRadiusTR;
    LTextureView *borderRadiusBR;
    LTextureView *borderRadiusBL;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerLeaveEvent() override;

    Output *output = nullptr;
    Float32 visiblePercent = 0.f;
    LAnimation *anim = nullptr;
};

#endif // DOCK_H
