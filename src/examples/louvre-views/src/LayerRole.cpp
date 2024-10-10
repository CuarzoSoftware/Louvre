#include "Global.h"
#include "LayerRole.h"
#include "Surface.h"
#include "Compositor.h"
#include "Output.h"

LayerRole::LayerRole(const void *params) noexcept :
    LLayerRole(params),
    isShelf(scope() == "cuarzo-shelf")
{
    if (isShelf)
        G::setShelf(client());

    static_cast<Surface*>(surface())->getView()->setParent(&G::compositor()->overlayLayer);
}

void LayerRole::configureRequest()
{
    LLayerRole::configureRequest();

    if (isShelf && exclusiveOutput())
        static_cast<Output*>(exclusiveOutput())->shelf.reset(this);
}
