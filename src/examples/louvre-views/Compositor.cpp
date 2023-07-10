#include <Compositor.h>
#include <Output.h>
#include <Surface.h>
#include <Pointer.h>
#include <LLayerView.h>
#include <LAnimation.h>

Compositor::Compositor():LCompositor()
{
    scene.setClearColor(0.1f, 0.1f, 0.5f);
    scene.mainView().enableClipping(true);
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
