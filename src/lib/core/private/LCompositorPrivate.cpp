#include "LCursor.h"
#include "LPainter.h"
#include <dlfcn.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <LLog.h>
#include <EGL/egl.h>

void LCompositor::LCompositorPrivate::processRemovedGlobals()
{
    list<RemovedGlobal*>::iterator it;
    for (it = removedGlobals.begin(); it != removedGlobals.end(); it++)
    {
        RemovedGlobal *rg = *it;

        if (rg->iters >= LOUVRE_GLOBAL_ITERS_BEFORE_DESTROY)
        {
            wl_global_destroy(rg->global);
            it = removedGlobals.erase(it);
            delete rg;
        }
        else
            rg->iters++;
    }
}

void LCompositor::LCompositorPrivate::removeGlobal(wl_global *global)
{
    wl_global_remove(global);
    RemovedGlobal *rg = new RemovedGlobal();
    rg->global = global;
    rg->iters = 0;
    removedGlobals.push_back(rg);
}

static wl_iterator_result resourceDestroyIterator(wl_resource *resource, void*)
{
    wl_resource_destroy(resource);
    return WL_ITERATOR_CONTINUE;
}

static void clientDisconnectedEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);

    wl_client *client = (wl_client*)data;

    wl_client_for_each_resource(client, resourceDestroyIterator, NULL);

    LClient *disconnectedClient = nullptr;

    // Remove client from the compositor list
    for (LClient *wClient: LCompositor::compositor()->clients())
    {
        if (wClient->client() == client)
        {
            disconnectedClient = wClient;
            break;
        }
    }

    if (disconnectedClient == nullptr)
        return;

    LCompositor::compositor()->destroyClientRequest(disconnectedClient);
    LCompositor::compositor()->imp()->clients.remove(disconnectedClient);

    delete disconnectedClient;
}

static void clientConnectedEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);

    wl_client *client = (wl_client*)data;

    LClient::Params *params = new LClient::Params;
    params->client = client;
    params->compositor = LCompositor::compositor();

    // Let the developer create his own client implementation
    LClient *newClient =  LCompositor::compositor()->createClientRequest(params);

    // Listen for client disconnection
    wl_client_get_destroy_listener(client, &clientDisconnectedEvent);

    // Append client to the compositor list
    LCompositor::compositor()->imp()->clients.push_back(newClient);
}

bool LCompositor::LCompositorPrivate::initWayland()
{
    unitWayland();

    // Create a new Wayland display
    display = wl_display_create();

    if (!display)
    {
        LLog::fatal("[compositor] Unable to create Wayland display.\n");
        return false;
    }

    const char *socket = getenv("LOUVRE_WAYLAND_DISPLAY");

    if (socket)
    {
        int socketFd = wl_display_add_socket(display, socket);

        if (socketFd == -1)
        {
            LLog::error("[compositor] Failed to add custom socket %s. Trying wl_display_add_socket_auto instead.", socket);
            goto useAutoSocket;
        }

        wl_display_add_socket_fd(display, socketFd);
    }
    else
    {
        useAutoSocket:

        // Use wayland-n socket by default
        socket = wl_display_add_socket_auto(display);

        if (!socket)
        {
            LLog::fatal("[compositor] Failed to add auto socket %s.", socket);
            return false;
        }
    }

    if (!compositor->createGlobalsRequest())
    {
        LLog::fatal("[compositor] Failed to create globals.");
        return false;
    }

    eventLoop = wl_display_get_event_loop(display);
    fdSet.fd = wl_event_loop_get_fd(eventLoop);

    // Listen for client connections
    clientConnectedListener.notify = &clientConnectedEvent;
    wl_display_add_client_created_listener(display, &clientConnectedListener);

    return true;
}

void LCompositor::LCompositorPrivate::unitWayland()
{
    if (display)
    {
        wl_display_destroy(display);
        display = nullptr;
    }
}

void LCompositor::LCompositorPrivate::uinitCompositor()
{
    state = CompositorState::Uninitializing;
    unitGraphicBackend();
    unitSeat();
    unitWayland();
    state = CompositorState::Uninitialized;
}

bool LCompositor::LCompositorPrivate::initGraphicBackend()
{
    unitGraphicBackend();

    eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress ("eglBindWaylandDisplayWL");

    if (!graphicBackend)
    {
        LLog::warning("[compositor] User did not load a graphic backend. Trying the DRM backend...");

        testDRMBackend:

        if (!loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendDRM.so"))
        {
            LLog::error("[compositor] Failed to load the DRM backend. Trying the X11 backend...");

            testX11Backend:

            if (!loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendX11.so"))
            {
                LLog::fatal("[compositor] No graphic backend found. Stopping compositor...");
                return false;
            }
        }
        else
        {
            if (!graphicBackend->initialize(compositor))
            {
                dlclose(graphicBackendHandle);
                graphicBackendHandle = nullptr;
                graphicBackend = nullptr;

                LLog::error("[compositor] Could not initialize the DRM backend. Trying the X11 backend...");
                goto testX11Backend;
            }
        }
    }
    else
    {
        if (!graphicBackend->initialize(compositor))
        {
            dlclose(graphicBackendHandle);
            graphicBackendHandle = nullptr;
            graphicBackend = nullptr;

            LLog::error("[compositor] Could not initialize the user defined backend. Trying the DRM backend...");
            goto testDRMBackend;
        }
    }

    LLog::debug("[compositor] Graphic backend initialized successfully.");
    isGraphicBackendInitialized = true;

    mainEGLDisplay = graphicBackend->getAllocatorEGLDisplay(compositor);
    mainEGLContext = graphicBackend->getAllocatorEGLContext(compositor);

    eglMakeCurrent(eglDisplay(),
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   eglContext());

    if (eglBindWaylandDisplayWL)
        eglBindWaylandDisplayWL(eglDisplay(), display);

    painter = new LPainter();
    cursor = new LCursor(compositor);
    compositor->cursorInitialized();

    return true;
}

void LCompositor::LCompositorPrivate::unitGraphicBackend()
{
    if (cursor)
    {
        delete cursor;
        cursor = nullptr;
    }

    if (painter)
    {
        delete painter;
        painter = nullptr;
    }

    mainEGLDisplay = EGL_NO_DISPLAY;
    mainEGLContext = EGL_NO_CONTEXT;

    eglMakeCurrent(EGL_NO_DISPLAY,
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    if (isGraphicBackendInitialized && graphicBackend)
        graphicBackend->uninitialize(compositor);

    isGraphicBackendInitialized = false;
}

bool LCompositor::LCompositorPrivate::initSeat()
{
    unitSeat();

    // Ask the developer to return a LSeat
    LSeat::Params seatParams;
    seatParams.compositor = LCompositor::compositor();
    seat = LCompositor::compositor()->createSeatRequest(&seatParams);
    return true;
}

void LCompositor::LCompositorPrivate::unitSeat()
{
    if (seat)
    {
        delete seat;
        seat = nullptr;
    }
}
