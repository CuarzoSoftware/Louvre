#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LLayerView.h>
#include <LCompositor.h>
#include <LScene.h>
#include <LView.h>

using namespace Louvre;

class Compositor : public LCompositor
{
public:
    Compositor();
    ~Compositor();

    void initialized() override;

    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;
    LPointer *createPointerRequest(LPointer::Params *params) override;

    LScene *scene;
    LLayerView *backgroundLayer;
    LLayerView *surfacesLayer;
    LLayerView *overlayLayer;
    LLayerView *hiddenCursorsLayer;
};

#endif // COMPOSITOR_H
