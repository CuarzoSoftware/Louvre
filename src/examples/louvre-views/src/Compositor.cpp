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
#include "TextRenderer.h"
#include "Topbar.h"

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
    G::loadFonts();

    clockTimer = wl_event_loop_add_timer(LCompositor::eventLoop(), &Compositor::timerCallback, this);
    wl_event_source_timer_update(clockTimer, 1);

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

Int32 Compositor::timerCallback(void *)
{
    if (G::font()->regular)
    {
        char text[128];
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(text, sizeof(text), "%a %b %d, %I:%M %p", timeinfo);

        LTexture *newClockTexture = G::font()->regular->renderText(text, 22);

        if (newClockTexture)
        {
            for (Output *o : G::outputs())
            {
                if (o->topbar && o->topbar->clock)
                {
                    o->topbar->clock->setTexture(newClockTexture);
                    o->topbar->update();
                }
            }

            if (G::compositor()->clockTexture)
            {
                delete G::compositor()->clockTexture;
                G::compositor()->clockTexture = newClockTexture;
            }
        }
    }

    wl_event_source_timer_update(G::compositor()->clockTimer, millisecondsUntilNextMinute() + 1500);
    return 0;
}

Int32 Compositor::millisecondsUntilNextMinute()
{
    time_t rawtime;
    struct tm *timeinfo;
    struct timespec spec;

    // Get the current time
    clock_gettime(CLOCK_REALTIME, &spec);
    rawtime = spec.tv_sec;
    timeinfo = localtime(&rawtime);

    // Calculate the number of seconds until the next minute
    int secondsUntilNextMinute = 60 - timeinfo->tm_sec;

    // Calculate the number of milliseconds until the next minute
    int msUntilNextMinute = secondsUntilNextMinute * 1000 - spec.tv_nsec / 1000000;

    return msUntilNextMinute;
}
