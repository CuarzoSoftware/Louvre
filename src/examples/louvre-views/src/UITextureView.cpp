#include <LPainter.h>
#include <LFramebuffer.h>
#include "Global.h"
#include "UITextureView.h"

UITextureView::UITextureView(UInt32 textureIndex, LView *parent) :
    Louvre::LTextureView(nullptr, parent),
    textureIndex { textureIndex }
{
    G::setTexViewConf(this, textureIndex);
}

void UITextureView::setTextureIndex(UInt32 textureIndex)
{
    G::setTexViewConf(this, textureIndex);
    this->textureIndex = textureIndex;
}
