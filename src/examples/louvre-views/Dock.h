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

    void update();
    void show();
    void hide();
    void handleCursorMovement();

    Output *output() const;

    LLayerView dockClipping = LLayerView(this);
    LSceneView *dockScene;
    LSolidColorView *dockBackground;
    LLayerView *itemsContainer;

    LTextureView *borderRadiusTL;
    LTextureView *borderRadiusTR;
    LTextureView *borderRadiusBR;
    LTextureView *borderRadiusBL;

private:
    Output *m_output;
    Float32 m_visiblePercent = 1.f;
    Int32 m_padding = 10;
    Int32 m_spacing = 10;
};

#endif // DOCK_H
