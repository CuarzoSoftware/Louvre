#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <LLayerView.h>
#include <LTextureView.h>
#include <LSolidColorView.h>

using namespace Louvre;

class Tooltip : LLayerView
{
public:
    Tooltip();

    LSolidColorView center;
    LTextureView decoration[8];
    LTextureView arrow;
    LTextureView label;

    LView *targetView = nullptr;

    void setText(const char *text);
    void show(Int32 x, Int32 y);
    void hide();

    LPoint point;

    void update();

    bool nativeMapped() const override;
};

#endif // TOOLTIP_H
