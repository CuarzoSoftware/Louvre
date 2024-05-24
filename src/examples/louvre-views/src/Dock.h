#ifndef DOCK_H
#define DOCK_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include <LAnimation.h>
#include "Global.h"
#include "UITextureView.h"

using namespace Louvre;

class Output;

class Dock final : public LLayerView
{
public:
    Dock(Output *output);

    void initialize() noexcept;
    void uninitialize() noexcept;
    void update();
    void show();
    void hide();

    // Dock textures container
    LLayerView dockContainer { this };

    // Dock textures
    UITextureView dockLeft   { &dockContainer };
    UITextureView dockCenter { &dockContainer };
    UITextureView dockRight  { &dockContainer };

    // Container for apps and minimized windows
    LLayerView appsContainer { &dockContainer };
    LSolidColorView separator { 0.f, 0.f, 0.f, 0.2f, &appsContainer };
    LLayerView itemsContainer { &dockContainer };

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerMoveEvent(const LPointerMoveEvent &) override;
    void pointerLeaveEvent(const LPointerLeaveEvent &) override;

    // Output of this dock
    LWeak<Output> output;

    // 0.f = HIDDEN, 1.0f = VISIBLE
    Float32 visiblePercent = 0.f;

    // HIDE/SHOW animation
    LAnimation anim;

    // Number of pointerMoveEvent() calls before show() is called
    UInt32 showResistance = 6;
    UInt32 showResistanceCount = 0;

    bool initialized { false };

    bool nativeMapped() const noexcept override;
};

#endif // DOCK_H
