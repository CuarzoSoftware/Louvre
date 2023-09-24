#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LAnimationPrivate.h>
#include <LTime.h>
#include <LLog.h>
#include <EGL/egl.h>
#include <dlfcn.h>

void LCompositor::LCompositorPrivate::processRemovedGlobals()
{
    std::list<RemovedGlobal*>::iterator it;
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

static wl_iterator_result resourceDestroyIterator(wl_resource *resource, void *data)
{
    wl_resource **lastCreatedResource = (wl_resource **)data;
    *lastCreatedResource = resource;
    return WL_ITERATOR_CONTINUE;
}

static void clientDisconnectedEvent(wl_listener *listener, void *data)
{
    delete listener;
    LCompositor *compositor = LCompositor::compositor();
    wl_client *client = (wl_client*)data;

    wl_resource *lastCreatedResource = NULL;

    while (1)
    {
        lastCreatedResource = NULL;
        wl_client_for_each_resource(client, resourceDestroyIterator, &lastCreatedResource);

        if (lastCreatedResource == NULL)
            break;
        else
            wl_resource_destroy(lastCreatedResource);
    }

    LClient *disconnectedClient = compositor->getClientFromNativeResource(client);

    if (disconnectedClient == nullptr)
        return;

    compositor->destroyClientRequest(disconnectedClient);
    compositor->imp()->clients.erase(disconnectedClient->imp()->compositorLink);
    delete disconnectedClient;
}

static void clientConnectedEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);
    LCompositor *compositor = LCompositor::compositor();
    wl_client *client = (wl_client*)data;
    LClient::Params *params = new LClient::Params;
    params->client = client;

    wl_listener *destroyListener = new wl_listener();
    destroyListener->notify = clientDisconnectedEvent;

    // Listen for client disconnection
    wl_client_add_destroy_listener(client, destroyListener);

    // Let the developer create his own client implementation
    LClient *newClient =  compositor->createClientRequest(params);

    // Append client to the compositor list
    compositor->imp()->clients.push_back(newClient);
    newClient->imp()->compositorLink = std::prev(compositor->imp()->clients.end());
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
    fdSet[0].fd = wl_event_loop_get_fd(eventLoop);

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
    cursor = new LCursor();
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

bool LCompositor::LCompositorPrivate::loadGraphicBackend(const char *path)
{
    graphicBackendHandle = dlopen(path, RTLD_LAZY);

    if (!graphicBackendHandle)
    {
        LLog::warning("[compositor] No graphic backend found at (%s)\n",path);
        return false;
    }

    LGraphicBackendInterface *(*getAPI)() = (LGraphicBackendInterface *(*)())dlsym(graphicBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::error("[compositor] Failed to load graphic backend (%s)\n",path);
        dlclose(graphicBackendHandle);
        return false;
    }

    graphicBackend = getAPI();

    if (graphicBackend)
        LLog::debug("[compositor] Graphic backend loaded successfully (%s).", path);

    return true;
}

bool LCompositor::LCompositorPrivate::loadInputBackend(const char *path)
{
    inputBackendHandle = dlopen(path, RTLD_LAZY);

    if (!inputBackendHandle)
    {
        LLog::warning("[compositor] No input backend found at (%s).",path);
        return false;
    }

    LInputBackendInterface *(*getAPI)() = (LInputBackendInterface *(*)())dlsym(inputBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::warning("[compositor] Failed to load input backend (%s).",path);
        dlclose(inputBackendHandle);
        return false;
    }

    inputBackend = getAPI();

    if (inputBackend)
        LLog::debug("[compositor] Input backend loaded successfully (%s).", path);

    return true;
}

void LCompositor::LCompositorPrivate::insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert)
{
    surfaces.erase(surfaceToInsert->imp()->compositorLink);

    if (prevSurface == surfaces.back())
    {
        surfaces.push_back(surfaceToInsert);
        surfaceToInsert->imp()->compositorLink = std::prev(surfaces.end());
    }
    else
        surfaceToInsert->imp()->compositorLink = surfaces.insert(std::next(prevSurface->imp()->compositorLink), surfaceToInsert);

    surfaceToInsert->orderChanged();
}

void LCompositor::LCompositorPrivate::insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert)
{
    surfaces.erase(surfaceToInsert->imp()->compositorLink);
    surfaceToInsert->imp()->compositorLink = surfaces.insert(nextSurface->imp()->compositorLink, surfaceToInsert);
    surfaceToInsert->orderChanged();
}

bool LCompositor::LCompositorPrivate::runningAnimations()
{
    for (LAnimation *anim : animations)
        if (anim->imp()->running || anim->imp()->pendingDestroy)
            return true;

    return false;
}

void LCompositor::LCompositorPrivate::processAnimations()
{
    auto it = animations.begin();

    while (it != animations.end())
    {
        LAnimation *a = *it;

        if (a->imp()->pendingDestroy)
        {
            it = animations.erase(it);
            delete a;
            continue;
        }

        if (!a->imp()->running)
        {
            it++;
            continue;
        }

        Int64 elapsed;

        elapsed = (Int64)LTime::ms() - (Int64)a->imp()->beginTime;

        if (elapsed > (Int64)a->imp()->duration)
            elapsed = a->imp()->duration;

        a->imp()->value = (Float32)elapsed/(Float32)a->imp()->duration;

        if (a->imp()->onUpdate)
            a->imp()->onUpdate(a);

        if (a->imp()->value == 1.f)
        {
            if (a->imp()->onFinish)
                a->imp()->onFinish(a);

            a->imp()->running = false;

            if (a->imp()->destroyOnFinish)
            {
                it = animations.erase(it);
                delete a;
                continue;
            }
        }

        it++;
    }
}

void LCompositor::LCompositorPrivate::destroyPendingRenderBuffers(std::thread::id *id)
{
    std::thread::id threadId = std::this_thread::get_id();

    if (id != nullptr)
        threadId = *id;

    ThreadData &threadData = threadsMap[threadId];

    while (!threadData.renderBuffersToDestroy.empty())
    {
        glDeleteTextures(1, &threadData.renderBuffersToDestroy.back().textureId);
        glDeleteFramebuffers(1, &threadData.renderBuffersToDestroy.back().framebufferId);
        threadData.renderBuffersToDestroy.pop_back();
    }
}

void LCompositor::LCompositorPrivate::addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::LRenderBufferPrivate::ThreadData &data)
{
    ThreadData &threadData = threadsMap[thread];
    threadData.renderBuffersToDestroy.push_back(data);
}

void LCompositor::LCompositorPrivate::lock()
{
    queueMutex.lock();
    threadsQueue.push_back(std::this_thread::get_id());
    queueMutex.unlock();

retry:
    renderMutex.lock();

    queueMutex.lock();
    if (threadsQueue.front() != std::this_thread::get_id())
    {
        renderMutex.unlock();
        queueMutex.unlock();
        goto retry;
    }
    queueMutex.unlock();
}

void LCompositor::LCompositorPrivate::unlock()
{
    renderMutex.unlock();

    queueMutex.lock();
    threadsQueue.pop_front();
    queueMutex.unlock();
}

void LCompositor::LCompositorPrivate::unlockPoll()
{
    if (pollUnlocked)
        return;

    pollUnlocked = true;
    uint64_t event_value = 1;
    ssize_t n = write(fdSet[1].fd, &event_value, sizeof(event_value));
    L_UNUSED(n);
}
