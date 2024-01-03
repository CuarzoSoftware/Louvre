#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <LLayerView.h>
#include <LSolidColorView.h>

#include "UITextureView.h"

using namespace Louvre;

class Tooltip : LLayerView
{
public:
    Tooltip();

    LSolidColorView center;

    UITextureView decoT;
    UITextureView decoR;
    UITextureView decoB;
    UITextureView decoL;
    UITextureView decoTL;
    UITextureView decoTR;
    UITextureView decoBR;
    UITextureView decoBL;

    UITextureView arrow;
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
