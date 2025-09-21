#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <LLayerView.h>
#include <LSolidColorView.h>

#include "Global.h"
#include "UITextureView.h"

using namespace Louvre;

class Tooltip : LLayerView {
 public:
  Tooltip();

  LTextureView label;
  LPoint globalPos;

  // Dock item
  LView *targetView{nullptr};

  LSolidColorView center{0.97f, 0.97f, 0.97f, 1.f, this};
  UITextureView decoT{G::TooltipT, this};
  UITextureView decoR{G::TooltipR, this};
  UITextureView decoB{G::TooltipB, this};
  UITextureView decoL{G::TooltipL, this};
  UITextureView decoTL{G::TooltipTL, this};
  UITextureView decoTR{G::TooltipTR, this};
  UITextureView decoBR{G::TooltipBR, this};
  UITextureView decoBL{G::TooltipBL, this};
  UITextureView arrow{G::TooltipArrow, this};

  void setText(const char *text);
  void show(Int32 x, Int32 y);
  void hide();
  void update();

  bool nativeMapped() const noexcept override;
};

#endif  // TOOLTIP_H
