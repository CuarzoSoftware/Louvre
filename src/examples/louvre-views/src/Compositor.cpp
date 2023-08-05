#include <LLayerView.h>
#include <LAnimation.h>
#include <LTextureView.h>
#include <LLog.h>

#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Surface.h"
#include "Pointer.h"
#include "Keyboard.h"
#include "Toplevel.h"

Compositor::Compositor():LCompositor()
{
    scene = new LScene();

    // Set black as tue clear color which will be visible if
    // no wallpaper is loaded
    scene->mainView()->setClearColor(0.f, 0.f, 0.f, 1.f);

    // Add layers to the scene in the correct order
    backgroundLayer = new LLayerView(scene->mainView());
    surfacesLayer = new LLayerView(scene->mainView());
    fullscreenLayer = new LLayerView(scene->mainView());
    overlayLayer = new LLayerView(scene->mainView());
}

Compositor::~Compositor()
{
    delete overlayLayer;
    delete fullscreenLayer;
    delete surfacesLayer;
    delete backgroundLayer;
    delete scene;
}

void Compositor::initialized()
{
    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap(NULL, NULL, "latam", NULL);

    G::loadDockTextures();
    G::loadCursors();
    G::loadToplevelTextures();

    Int32 totalWidth = 0;

    // Initialize and arrange outputs (screens) left to right
    for (LOutput *output : *seat()->outputs())
    {
        // Set scale 2 to HiDPI screens
        output->setScale(output->dpi() >= 120 ? 2 : 1);
        output->setPos(LPoint(totalWidth, 0));
        totalWidth += output->size().w();
        compositor()->addOutput(output);
        output->repaint();
    }
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

LKeyboard *Compositor::createKeyboardRequest(LKeyboard::Params *params)
{
    return new Keyboard(params);
}

LToplevelRole *Compositor::createToplevelRoleRequest(LToplevelRole::Params *params)
{
    return new Toplevel(params);
}
