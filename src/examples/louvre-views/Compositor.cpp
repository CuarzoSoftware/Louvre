#include <Compositor.h>
#include <Output.h>
#include <Surface.h>
#include <Pointer.h>
#include <LLayerView.h>
#include <LAnimation.h>

Compositor::Compositor():LCompositor()
{
    scene = new LScene();
    scene->setClearColor(0.1f, 0.1f, 0.5f);
    backgroundLayer = new LLayerView(&scene->mainView());
    surfacesLayer = new LLayerView(&scene->mainView());
    overlayLayer = new LLayerView(&scene->mainView());
    hiddenCursorsLayer = new LLayerView(&scene->mainView());
}

Compositor::~Compositor()
{
    delete hiddenCursorsLayer;
    delete overlayLayer;
    delete surfacesLayer;
    delete scene;
    delete backgroundLayer;
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
