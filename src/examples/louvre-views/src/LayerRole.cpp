#include "LayerRole.h"

#include "Compositor.h"
#include "Global.h"
#include "Output.h"
#include "Surface.h"

LayerRole::LayerRole(const void *params) noexcept
    : LLayerRole(params), isShelf(scope() == "cuarzo-shelf") {
  if (isShelf) {
    G::setShelf(client());
    static_cast<Surface *>(surface())->getView()->setParent(
        &G::compositor()->overlayLayer);
  }
}

void LayerRole::configureRequest() {
  LLayerRole::configureRequest();

  if (isShelf && exclusiveOutput())
    static_cast<Output *>(exclusiveOutput())->shelf.reset(this);
}
