#include "LPainter.h"
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LOutputManagerPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCursorPrivate.h>

#include <protocols/Wayland/GOutput.h>

#include <LNamespaces.h>
#include <LWayland.h>
#include <LPopupRole.h>

#include <stdio.h>
#include <sys/poll.h>
#include <thread>
#include <unistd.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDNDManager.h>
#include <dlfcn.h>
#include <LLog.h>


using namespace Louvre::Protocols::Wayland;

LCompositor::LCompositor()
{
    LLog::init();
    m_imp = new LCompositorPrivate();
    imp()->compositor = this;

    LWayland::initWayland(this);

    // Store the main thread id for later use (in LCursor)
    imp()->threadId = std::this_thread::get_id();

}

bool LCompositor::graphicBackendInitialized() const
{
    return imp()->graphicBackendInitialized;
}

bool LCompositor::inputBackendInitialized() const
{
    return imp()->inputBackendInitialized;
}

bool LCompositor::loadGraphicBackend(const char *path)
{
    return imp()->loadGraphicBackend(path);
}

bool LCompositor::loadInputBackend(const char *path)
{
    return imp()->loadInputBackend(path);
}

Int32 LCompositor::globalScale() const
{
    return imp()->globalScale;
}

LCompositor::~LCompositor()
{
    delete m_imp;
}

int LCompositor::start()
{    
    if (imp()->started)
    {
        LLog::warning("Attempting to start a compositor already running. Ignoring...");
        return EXIT_FAILURE;
    }

    // Ask the developer to return a LSeat
    LSeat::Params seatParams;
    seatParams.compositor = this;
    imp()->seat = createSeatRequest(&seatParams);
    LWayland::setSeat(imp()->seat);

    if (!imp()->graphicBackend)
    {
        LLog::warning("User did not load a graphic backend. Trying the DRM backend...");

        testDRMBackend:

        if (!loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendDRM.so"))
        {
            LLog::error("Failed to load the DRM backend. Trying the X11 backend...");

            testX11Backend:

            if (!loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendX11.so"))
            {
                LLog::fatal("No graphic backend found. Stopping compositor...");
                return EXIT_FAILURE;
            }

        }
        else
        {
            if (!imp()->graphicBackend->initialize(this))
            {
                dlclose(imp()->graphicBackendHandle);
                imp()->graphicBackendHandle = nullptr;
                imp()->graphicBackend = nullptr;

                LLog::error("Could not initialize the DRM backend. Trying the X11 backend...");
                goto testX11Backend;
            }
        }
    }
    else
    {
        if (!imp()->graphicBackend->initialize(this))
        {
            dlclose(imp()->graphicBackendHandle);
            imp()->graphicBackendHandle = nullptr;
            imp()->graphicBackend = nullptr;

            LLog::error("Could not initialize the user defined backend. Trying the DRM backend...");
            goto testDRMBackend;
        }
    }

    LLog::debug("Graphic backend initialized successfully.");
    imp()->graphicBackendInitialized = true;

    eglMakeCurrent(imp()->graphicBackend->getAllocatorEGLDisplay(this),
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   imp()->graphicBackend->getAllocatorEGLContext(this));

    LWayland::bindEGLDisplay(imp()->graphicBackend->getAllocatorEGLDisplay(this));

    imp()->painter = new LPainter();
    imp()->cursor = new LCursor(this);
    cursorInitialized();

    LOutputManager::Params outputManagerParams;
    outputManagerParams.compositor = this;
    imp()->outputManager = createOutputManagerRequest(&outputManagerParams);

    if (!imp()->inputBackend)
    {
        LLog::warning("User did not load an input backend. Trying the Libinput backend...");

        if (!loadInputBackend("/usr/etc/Louvre/backends/libLInputBackendLibinput.so"))
        {
            LLog::fatal("No input backend found. Stopping compositor...");
            return EXIT_FAILURE;
        }
    }

    if (!imp()->inputBackend->initialize(seat()))
    {
        LLog::fatal("Failed to initialize input backend. Stopping compositor...");
        return EXIT_FAILURE;
    }

    LLog::debug("Input backend initialized successfully.");
    imp()->inputBackendInitialized = true;


    seat()->initialized();

    initialized();

    // Init wayland
    imp()->started = true;
    LWayland::runLoop();

    return EXIT_SUCCESS;
}

void LCompositor::finish()
{
    abort();

    if (imp()->started)
    {
        if (imp()->inputBackendInitialized)
            imp()->inputBackend->uninitialize(seat());

        if (imp()->graphicBackendInitialized)
            imp()->graphicBackend->uninitialize(this);
    }
}

void LCompositor::LCompositorPrivate::raiseChildren(LSurface *surface)
{            
    surfaces.splice( surfaces.end(), surfaces, surface->imp()->compositorLink);

    surface->raised();

    // Rise its children
    for (LSurface *children : surface->children())
        raiseChildren(children);
}

void LCompositor::raiseSurface(LSurface *surface)
{
    if (surface->subsurface())
    {
        raiseSurface(surface->parent());
        return;
    }

    imp()->raiseChildren(surface);
}

bool LCompositor::LCompositorPrivate::loadGraphicBackend(const char *path)
{
    graphicBackendHandle = dlopen(path, RTLD_LAZY);

    if (!graphicBackendHandle)
    {
        LLog::warning("No graphic backend found at (%s)\n",path);
        return false;
    }

    LGraphicBackendInterface *(*getAPI)() = (LGraphicBackendInterface *(*)())dlsym(graphicBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::warning("Failed to load graphic backend (%s)\n",path);
        dlclose(graphicBackendHandle);
        return false;
    }

    graphicBackend = getAPI();

    if (graphicBackend)
        LLog::debug("Graphic backend loaded successfully (%s).", path);

    return true;
}

bool LCompositor::LCompositorPrivate::loadInputBackend(const char *path)
{
    inputBackendHandle = dlopen(path, RTLD_LAZY);

    if (!inputBackendHandle)
    {
        LLog::warning("No input backend found at (%s).",path);
        return false;
    }

    LInputBackendInterface *(*getAPI)() = (LInputBackendInterface *(*)())dlsym(inputBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::warning("Failed to load input backend (%s).",path);
        dlclose(inputBackendHandle);
        return false;
    }

    inputBackend = getAPI();

    if (inputBackend)
        LLog::debug("Input backend loaded successfully (%s).", path);


    return true;
}

void LCompositor::LCompositorPrivate::insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert)
{
    surfaces.splice(std::next(prevSurface->imp()->compositorLink), surfaces, surfaceToInsert->imp()->compositorLink);
}

void LCompositor::LCompositorPrivate::insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert)
{
    surfaces.splice(nextSurface->imp()->compositorLink, surfaces, surfaceToInsert->imp()->compositorLink);
}

void LCompositor::LCompositorPrivate::updateGlobalScale()
{
    Int32 maxFound = 1;

    for (LOutput *o : outputs)
    {
        if (o->scale() > maxFound)
            maxFound = o->scale();
    }

    if (maxFound != globalScale)
    {
        Int32 oldScale = globalScale;
        globalScale = maxFound;

        for (LOutput *o : outputs)
            o->imp()->globalScaleChanged(oldScale, globalScale);

        for (LSurface *s : surfaces)
            s->imp()->globalScaleChanged(oldScale, globalScale);

        compositor->globalScaleChanged(oldScale, globalScale);

        if (cursor)
            cursor->imp()->globalScaleChanged(oldScale, globalScale);
    }

}


LCursor *LCompositor::cursor() const
{
    return imp()->cursor;
}

LSeat *LCompositor::seat() const
{
    return imp()->seat;
}

void LCompositor::repaintAllOutputs()
{
    for (list<LOutput*>::iterator it = imp()->outputs.begin(); it != imp()->outputs.end(); ++it)
        (*it)->repaint();
}

bool LCompositor::addOutput(LOutput *output)
{
    // Verifica que no se haya añadido previamente
    for (LOutput *o : outputs())
        if (o == output)
            return true;

    imp()->outputs.push_back(output);

    if (!output->imp()->initialize(this))
    {
        LLog::error("[Compositor] Failed to initialize output %s.", output->name());
        imp()->outputs.remove(output);
        return false;
    }

    return true;
}

void LCompositor::removeOutput(LOutput *output)
{
    // Iteramos para verificar si efectivamente la salida estaba añadida
    for (list<LOutput*>::iterator it = imp()->outputs.begin(); it != imp()->outputs.end(); it++)
    {
        if (*it == output)
        {
            output->imp()->state = LOutput::PendingUninitialize;
            imp()->graphicBackend->uninitializeOutput(output);
            output->imp()->state = LOutput::Uninitialized;

            // La eliminamos de las listas de intersección de las superficies
            for (LSurface *s : surfaces())
                s->sendOutputLeaveEvent(output);

            // La eliminamos de la lista de salidas añadidas
            imp()->outputs.remove(output);

            // Eliminamos los globals wl_output de cada cliente
            for (LClient *c : clients())
            {
                for (GOutput *g : c->outputGlobals())
                {
                    if (output == g->output())
                    {
                        wl_resource_destroy(g->resource());
                        break;
                    }
                }
            }

            // Hacemos esto para eliminar el global de forma segura (ver LWayland:loop)
            wl_global_remove(output->imp()->global);

            LCompositorPrivate::RemovedOutputGlobal *g = new LCompositorPrivate::RemovedOutputGlobal();
            g->global = output->imp()->global;
            g->loopIterations = 0;
            imp()->removedOutputGobals.push_back(g);

            imp()->updateGlobalScale();

            cursor()->imp()->intersectedOutputs.remove(output);

            if (cursor()->imp()->output == output)
                cursor()->imp()->output = nullptr;

            return;
        }
    }
}

LOutputManager *LCompositor::outputManager() const
{
    return imp()->outputManager;
}

const list<LSurface *> &LCompositor::surfaces() const
{
    return imp()->surfaces;
}

const list<LOutput *> &LCompositor::outputs() const
{
    return imp()->outputs;
}

const list<LClient *> &LCompositor::clients() const
{
    return imp()->clients;
}

LClient *LCompositor::getClientFromNativeResource(wl_client *client)
{
    LClient *lClient = nullptr;

    for (LClient *c : clients())
    {
        if (c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    return lClient;
}

thread::id LCompositor::mainThreadId() const
{
    return imp()->threadId;
}
