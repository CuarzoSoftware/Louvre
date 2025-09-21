#include "DockItem.h"

#include <LCursor.h>
#include <LScene.h>
#include <LXCursor.h>

#include "Dock.h"
#include "Global.h"
#include "Surface.h"
#include "Tooltip.h"

DockItem::DockItem(class Surface *surface, Dock *dck) noexcept
    : LTextureView(surface->thumbnailTex, &dck->itemsContainer) {
  setUserData(DockItemType);
  surface->minimizedViews.push_back(this);
  this->dock = dck;
  this->surface = surface;
  setBufferScale(4);
  enableScaling(true);
  enableParentOpacity(false);
  enablePointerEvents(true);
  enableBlockPointer(false);
}

DockItem::~DockItem() { surface->minimizedViews.remove(this); }

void DockItem::pointerEnterEvent(const LPointerEnterEvent &) {
  setOpacity(0.8f);

  if (surface->toplevel()) {
    G::tooltip()->setText(surface->toplevel()->title().c_str());
    G::tooltip()->targetView = this;
    dock->update();
  } else
    G::tooltip()->hide();
}

void DockItem::pointerLeaveEvent(const LPointerLeaveEvent &) {
  setOpacity(1.f);
}

void DockItem::pointerButtonEvent(const LPointerButtonEvent &event) {
  if (event.button() != LPointerButtonEvent::Button::Left) return;

  if (event.state() == LPointerButtonEvent::Pressed)
    setOpacity(0.7f);
  else {
    surface->unminimize(this);
    G::tooltip()->hide();
  }
}
