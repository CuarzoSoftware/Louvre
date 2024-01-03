#ifndef UITEXTUREVIEW_H
#define UITEXTUREVIEW_H

#include <LTextureView.h>
#include "Global.h"

/* This texture view dynamically adjusts the texture scale during a paintEvent, ensuring alignment with the current framebuffer scale. */

class UITextureView : public Louvre::LTextureView
{
public:
    UITextureView(UInt32 textureIndex, LView *parent, Float32 scale = 2.f);
    void setTextureIndex(UInt32 textureIndex, Float32 scale = 2.f);
    void paintEvent(const Louvre::LTextureView::PaintEventParams &params) override;
    UInt32 textureIndex;
};

#endif // UITEXTUREVIEW_H
