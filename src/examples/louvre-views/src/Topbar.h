#ifndef TOPBAR_H
#define TOPBAR_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include "UITextureView.h"

class Output;

using namespace Louvre;

class Topbar : public LLayerView
{
public:
    Topbar(Output *output);
    ~Topbar();

    void update();

    Output *output;

    // White bar
    LSolidColorView background;

    // Louvre logo
    UITextureView logo;

    // Clock text
    LTextureView clock;

    // Current app title
    LTextureView appName;

    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerMoveEvent(const LPoint &) override;
};

#endif // TOPBAR_H
