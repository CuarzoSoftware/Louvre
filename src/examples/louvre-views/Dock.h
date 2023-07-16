#ifndef DOCK_H
#define DOCK_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include <LAnimation.h>
#include <Shared.h>

using namespace Louvre;

class Output;

class Dock : public LLayerView
{
public:
    Dock(Output *output);
    LSolidColorView background = LSolidColorView(1.f, 1.f, 1.f, 0.8f, this);
    void update();
    void show();
    void hide();
    void handleCursorMovement();

private:
    Output *m_output;
    Float32 m_visiblePercent = 1.f;
    Int32 m_padding = 10;
    Int32 m_spacing = 10;
};

#endif // DOCK_H
