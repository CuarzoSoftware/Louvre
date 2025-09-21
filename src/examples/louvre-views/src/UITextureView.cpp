#include "UITextureView.h"

#include <LFramebuffer.h>
#include <LPainter.h>

#include "Global.h"

UITextureView::UITextureView(LView *parent) noexcept
    : LTextureView(nullptr, parent) {}

UITextureView::UITextureView(UInt32 textureIndex, LView *parent)
    : Louvre::LTextureView(nullptr, parent), textureIndex{textureIndex} {
  G::setTexViewConf(this, textureIndex);
}

void UITextureView::setTextureIndex(UInt32 textureIndex) {
  G::setTexViewConf(this, textureIndex);
  this->textureIndex = textureIndex;
}
