#include "LPainter.h"
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LOutputManagerPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCursorPrivate.h>

#include <protocols/Wayland/private/GOutputPrivate.h>

#include <LNamespaces.h>
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

static LCompositor *s_compositor = nullptr;

LCompositor::LCompositor()
{
    LLog::init();
    m_imp = new LCompositorPrivate();
    imp()->compositor = this;
}

Louvre::LCompositor *LCompositor::compositor()
{
    return s_compositor;
}

bool LCompositor::isGraphicBackendInitialized() const
{
    return imp()->isGraphicBackendInitialized;
}

bool LCompositor::isInputBackendInitialized() const
{
    return imp()->isInputBackendInitialized;
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

LCompositor::CompositorState LCompositor::state() const
{
    return imp()->state;
}

LCompositor::~LCompositor()
{
    delete m_imp;
}

bool LCompositor::start()
{
    if (compositor())
    {
        LLog::warning("Compositor already running. Two Louvre compositors can not live in the same process.");
        return false;
    }

    s_compositor = this;

    if (state() != CompositorState::Uninitialized)
    {
        LLog::warning("Attempting to start a compositor already running. Ignoring...");
        return false;
    }

    imp()->threadId = std::this_thread::get_id();
    imp()->state = CompositorState::Initializing;

    imp()->initWayland();

    // Ask the developer to return a LSeat
    LSeat::Params seatParams;
    seatParams.compositor = this;
    imp()->seat = createSeatRequest(&seatParams);

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
                goto fail;
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
    imp()->isGraphicBackendInitialized = true;

    imp()->mainEGLDisplay = imp()->graphicBackend->getAllocatorEGLDisplay(this);
    imp()->mainEGLContext = imp()->graphicBackend->getAllocatorEGLContext(this);

    eglMakeCurrent(eglDisplay(),
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   eglContext());

    if (imp()->eglBindWaylandDisplayWL)
        imp()->eglBindWaylandDisplayWL(eglDisplay(), display());

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
            goto fail;
        }
    }

    if (!imp()->inputBackend->initialize(seat()))
    {
        LLog::fatal("Failed to initialize input backend. Stopping compositor...");
        goto fail;
    }

    LLog::debug("Input backend initialized successfully.");
    imp()->isInputBackendInitialized = true;

    seat()->initialized();

    imp()->state = CompositorState::Initialized;
    initialized();

    imp()->fdSet.events = POLLIN | POLLOUT | POLLHUP;
    imp()->fdSet.revents = 0;

    return true;

    fail:
    imp()->state = CompositorState::Uninitialized;
    return false;
}

Int32 LCompositor::processLoop(Int32 msTimeout)
{
    poll(&imp()->fdSet, 1, msTimeout);

    imp()->renderMutex.lock();
    seat()->imp()->dispatchSeat();
    imp()->processRemovedGlobals();

    // DND
    if (seat()->dndManager()->imp()->destDidNotRequestReceive >= 3)
        seat()->dndManager()->cancel();

    if (seat()->dndManager()->imp()->dropped && seat()->dndManager()->imp()->destDidNotRequestReceive < 3)
        seat()->dndManager()->imp()->destDidNotRequestReceive++;

    wl_event_loop_dispatch(imp()->eventLoop, 0);
    flushClients();

    cursor()->imp()->textureUpdate();
    imp()->renderMutex.unlock();
}

void LCompositor::finish()
{
    abort();

    /* TODO:
    if (imp()->started)
    {
        if (imp()->isInputBackendInitialized)
            imp()->inputBackend->uninitialize(seat());

        if (imp()->isGraphicBackendInitialized)
            imp()->graphicBackend->uninitialize(this);
    }
    */
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

wl_display *LCompositor::display()
{
    return LCompositor::compositor()->imp()->display;
}

wl_event_source *LCompositor::addFdListener(int fd, void *userData, int (*callback)(int, unsigned int, void *), UInt32 flags)
{
    return wl_event_loop_add_fd(LCompositor::compositor()->imp()->eventLoop, fd, flags, callback, userData);
}

void LCompositor::removeFdListener(wl_event_source *source)
{
    wl_event_source_remove(source);
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
                        g->client()->imp()->outputGlobals.erase(g->imp()->clientLink);
                        g->imp()->output = nullptr;
                        break;
                    }
                }
            }

            imp()->removeGlobal(output->imp()->global);
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

UInt32 LCompositor::nextSerial()
{
    return wl_display_next_serial(LCompositor::compositor()->display());
}

EGLDisplay LCompositor::eglDisplay()
{
    return LCompositor::compositor()->imp()->mainEGLDisplay;
}

EGLContext LCompositor::eglContext()
{
    return LCompositor::compositor()->imp()->mainEGLContext;
}

void LCompositor::flushClients()
{
    wl_display_flush_clients(LCompositor::display());
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
