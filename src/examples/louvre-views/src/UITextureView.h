#ifndef UITEXTUREVIEW_H
#define UITEXTUREVIEW_H

#include <LTextureView.h>

using namespace Louvre;

class UITextureView : public LTextureView {
 public:
  UITextureView(LView *parent) noexcept;
  UITextureView(UInt32 textureIndex, LView *parent);
  void setTextureIndex(UInt32 textureIndex);
  UInt32 textureIndex;
};

#endif  // UITEXTUREVIEW_H
