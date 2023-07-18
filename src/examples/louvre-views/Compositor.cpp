#include <Compositor.h>
#include <Output.h>
#include <Surface.h>
#include <Pointer.h>
#include <LLayerView.h>
#include <LAnimation.h>
#include <Shared.h>
#include <string.h>
#include <LLog.h>

Compositor::Compositor():LCompositor()
{
    scene = new LScene();
    scene->mainView()->setClearColor(0.1f, 0.1f, 0.5f);
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
    Int32 circleRadius = 256;
    UChar8 circleBuffer[circleRadius*circleRadius*4];
    memset(circleBuffer, 0, sizeof(circleBuffer));

    for (Int32 x = 0; x < circleRadius; x++)
        for (Int32 y = 0; y < circleRadius; y++)
            circleBuffer[x*4 + y*circleRadius*4 + 3] = sqrt(x*x + y*y) < circleRadius ? 255 : 0;


    LTexture *circleTexture = new LTexture();
    circleTexture->setDataB(LSize(circleRadius, circleRadius), circleRadius*4, DRM_FORMAT_ARGB8888, circleBuffer);

    shared()->circleTextureTL = circleTexture->copyB(64, LRect(0, 0, -circleRadius, -circleRadius));
    shared()->circleTextureTR = circleTexture->copyB(64, LRect(0, 0,  circleRadius, -circleRadius));
    shared()->circleTextureBR = circleTexture->copyB(64, LRect(0, 0,  circleRadius,  circleRadius));
    shared()->circleTextureBL = circleTexture->copyB(64, LRect(0, 0, -circleRadius,  circleRadius));

    delete circleTexture;

    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap(NULL, NULL, "latam", NULL);
    arrangeOutputs();
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
