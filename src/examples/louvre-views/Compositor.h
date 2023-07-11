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

    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;
    LPointer *createPointerRequest(LPointer::Params *params) override;

    LScene scene;
    LLayerView backgroundLayer = LLayerView((LView*)&scene.mainView());
    LLayerView surfacesLayer = LLayerView((LView*)&scene.mainView());
    LLayerView overlayLayer = LLayerView((LView*)&scene.mainView());
    LLayerView cursorsLayer = LLayerView((LView*)&scene.mainView());

};

#endif // COMPOSITOR_H
