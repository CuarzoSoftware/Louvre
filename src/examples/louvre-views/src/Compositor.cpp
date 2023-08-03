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
    scene->mainView()->setClearColor(0.1f, 0.1f, 0.5f, 1.f);
    backgroundLayer = new LLayerView(scene->mainView());
    surfacesLayer = new LLayerView(scene->mainView());
    overlayLayer = new LLayerView(scene->mainView());
}

Compositor::~Compositor()
{
    delete overlayLayer;
    delete surfacesLayer;
    delete scene;
    delete backgroundLayer;
}

void Compositor::initialized()
{
    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap(NULL, NULL, "latam", NULL);

    G::loadDockTextures();
    G::loadCursors();
    G::loadToplevelTextures();

    Int32 totalWidth = 0;
    for (LOutput *output : *seat()->outputs())
    {
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
