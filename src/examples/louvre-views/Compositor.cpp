#include <Compositor.h>
#include <Output.h>
#include <Surface.h>
#include <Pointer.h>
#include <LLayerView.h>
#include <LAnimation.h>
#include <Global.h>
#include <LLog.h>
#include <LTextureView.h>

#include "Toplevel.h"

Compositor::Compositor():LCompositor()
{
    scene = new LScene();
    scene->mainView()->setClearColor(0.1f, 0.1f, 0.5f, 1.f);
    backgroundLayer = new LLayerView(scene->mainView());
    surfacesLayer = new LLayerView(scene->mainView());
    overlayLayer = new LLayerView(scene->mainView());
    hiddenCursorsLayer = new LLayerView(scene->mainView());
}

Compositor::~Compositor()
{
    delete hiddenCursorsLayer;
    delete overlayLayer;
    delete surfacesLayer;
    delete scene;
    delete backgroundLayer;
}

void Compositor::initialized()
{
    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap(NULL, NULL, "latam", NULL);

    G::createBorderRadiusTextures();

    for (LOutput *output : *seat()->outputs())
        compositor()->addOutput(output);

    // Arrange outputs from left to right
    G::arrangeOutputs();

    G::loadCursors();
}

LOutput *Compositor::createOutputRequest()
{
    return new Output();
}

LSurface *Compositor::createSurfaceRequest(LSurface::Params *params)
{
    return new Surface(params);
}

LPointer *Compositor::createPointerRequest(LPointer::Params *params)
{
    return new Pointer(params);
}

LToplevelRole *Compositor::createToplevelRoleRequest(LToplevelRole::Params *params)
{
    return new Toplevel(params);
}
