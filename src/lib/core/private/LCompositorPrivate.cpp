#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LAnimationPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <LClipboard.h>
#include <LKeyboard.h>
#include <LPointer.h>
#include <LTouch.h>
#include <LTime.h>
#include <LTimer.h>
#include <LLog.h>
#include <EGL/egl.h>
#include <dlfcn.h>
#include <string.h>

void LCompositor::LCompositorPrivate::processRemovedGlobals()
{
    for (std::size_t i = 0; i < removedGlobals.size();)
    {
        RemovedGlobal &rg { removedGlobals[i] };

        if (rg.iters >= LOUVRE_GLOBAL_ITERS_BEFORE_DESTROY)
        {
            wl_global_destroy(rg.global);
            removedGlobals[i] = std::move(removedGlobals.back());
            removedGlobals.pop_back();
        }
        else
        {
            rg.iters++;
            ++i;
        }
    }
}

void LCompositor::LCompositorPrivate::removeGlobal(wl_global *global)
{
    wl_global_remove(global);
    removedGlobals.emplace_back(global, 0);
}

static wl_iterator_result resourceDestroyIterator(wl_resource *resource, void *data)
{
    wl_resource **lastCreatedResource { (wl_resource **)data };
    *lastCreatedResource = resource;
    return WL_ITERATOR_CONTINUE;
}

static void clientDisconnectedEvent(wl_listener *listener, void *data)
{
    delete listener;
    LCompositor *compositor { LCompositor::compositor() };
    wl_client *client { (wl_client*)data };

    LClient *disconnectedClient { compositor->getClientFromNativeResource(client) };
    compositor->destroyClientRequest(disconnectedClient);

    wl_resource *lastCreatedResource { NULL };

    while (1)
    {
        lastCreatedResource = NULL;
        wl_client_for_each_resource(client, resourceDestroyIterator, &lastCreatedResource);

        if (lastCreatedResource == NULL)
            break;
        else
            wl_resource_destroy(lastCreatedResource);
    }

    LVectorRemoveOneUnordered(compositor->imp()->clients, disconnectedClient);
    delete disconnectedClient;
}

static void clientConnectedEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);
    LCompositor *compositor { LCompositor::compositor() };
    wl_client *client { (wl_client*)data };
    LClient::Params *params { new LClient::Params };
    params->client = client;

    wl_listener *destroyListener { new wl_listener() };
    destroyListener->notify = clientDisconnectedEvent;

    // Listen for client disconnection
    wl_client_add_destroy_listener(client, destroyListener);

    // Let the developer create his own client implementation
    LClient *newClient { compositor->createClientRequest(params) };

    // Append client to the compositor list
    compositor->imp()->clients.push_back(newClient);
}

bool LCompositor::LCompositorPrivate::initWayland()
{
    unitWayland();

    display = wl_display_create();

    if (!display)
    {
        LLog::fatal("[LCompositorPrivate::initWayland] Unable to create Wayland display.\n");
        return false;
    }

    const char *socket { getenv("LOUVRE_WAYLAND_DISPLAY") };

    if (socket)
    {
        int socketFd { wl_display_add_socket(display, socket) };

        if (socketFd == -1)
        {
            LLog::error("[LCompositorPrivate::initWayland] Failed to add custom socket %s. Trying wl_display_add_socket_auto instead.", socket);
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
            LLog::fatal("[LCompositorPrivate::initWayland] Failed to add auto socket.");
            return false;
        }
    }

    if (!compositor->createGlobalsRequest())
    {
        LLog::fatal("[LCompositorPrivate::initWayland] Failed to create globals.");
        return false;
    }

    eventLoop = wl_display_get_event_loop(display);

    compositor->imp()->events[2].events = EPOLLIN | EPOLLOUT;
    compositor->imp()->events[2].data.fd = wl_event_loop_get_fd(eventLoop);

    epoll_ctl(compositor->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor->imp()->events[2].data.fd,
              &compositor->imp()->events[2]);

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

void LCompositor::LCompositorPrivate::unitCompositor()
{
    state = CompositorState::Uninitializing;
    unitInputBackend(true);
    unitGraphicBackend(true);
    unitSeat();

    if (cursor)
    {
        delete cursor;
        cursor = nullptr;
    }

    unitWayland();

    if (epollFd != -1)
        close(epollFd);

    while (!oneShotTimers.empty())
        delete oneShotTimers.back();

    state = CompositorState::Uninitialized;
}

bool LCompositor::LCompositorPrivate::initGraphicBackend()
{
    unitGraphicBackend(false);

    // Check if there is a pre-loaded graphic backend
    if (graphicBackend)
    {
        if (!graphicBackend->backendInitialize())
        {
            dlclose(graphicBackendHandle);
            graphicBackendHandle = nullptr;
            graphicBackend = nullptr;

            LLog::error("[LCompositorPrivate::initGraphicBackend] Could not initialize pre-loaded backend.");
            goto loadEnvBackend;
        }
    }
    else
    {
        loadEnvBackend:

        std::filesystem::path backendsPath { getenvString("LOUVRE_BACKENDS_PATH") };
        std::filesystem::path backendName  { getenvString("LOUVRE_INPUT_BACKEND")};

        bool usingEnvs = !backendsPath.empty() || !backendName.empty();

        if (backendsPath.empty())
            backendsPath = LOUVRE_DEFAULT_BACKENDS_PATH;

        if (backendName.empty())
            backendName = LOUVRE_DEFAULT_GRAPHIC_BACKEND;

        std::filesystem::path backendPathName { backendsPath / "graphic" / backendName };
        backendPathName += ".so";

        retry:

        if (loadGraphicBackend(backendPathName))
        {
            if (!graphicBackend->backendInitialize())
            {
                dlclose(graphicBackendHandle);
                graphicBackendHandle = nullptr;
                graphicBackend = nullptr;
                LLog::error("[LCompositorPrivate::initGraphicBackend] Failed to initialize %s backend.", backendName.c_str());

                if (usingEnvs)
                {
                    usingEnvs = false;
                    backendPathName = std::filesystem::path(LOUVRE_DEFAULT_BACKENDS_PATH) / "graphic" / LOUVRE_DEFAULT_GRAPHIC_BACKEND;
                    backendPathName += ".so";
                    goto retry;
                }

                return false;
            }
        }
        else
        {
            LLog::error("[LCompositorPrivate::initGraphicBackend] Failed to load %s backend.", backendPathName.c_str());

            if (usingEnvs)
            {
                usingEnvs = false;
                backendPathName = std::filesystem::path(LOUVRE_DEFAULT_BACKENDS_PATH) / "graphic" / LOUVRE_DEFAULT_GRAPHIC_BACKEND;
                backendPathName += ".so";
                goto retry;
            }

            return false;
        }
    }

    LLog::debug("[LCompositorPrivate::initGraphicBackend] Graphic backend initialized successfully.");
    isGraphicBackendInitialized = true;

    mainEGLDisplay = graphicBackend->backendGetAllocatorEGLDisplay();
    mainEGLContext = graphicBackend->backendGetAllocatorEGLContext();

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

bool LCompositor::LCompositorPrivate::initInputBackend()
{
    unitInputBackend(false);

    // Check if there is a pre-loaded input backend
    if (inputBackend)
    {
        if (!inputBackend->backendInitialize())
        {
            dlclose(inputBackendHandle);
            inputBackendHandle = nullptr;
            inputBackend = nullptr;

            LLog::error("[LCompositorPrivate::initInputBackend] Could not initialize pre-loaded backend.");
            goto loadEnvBackend;
        }
    }
    else
    {
        loadEnvBackend:

        std::filesystem::path backendsPath { getenvString("LOUVRE_BACKENDS_PATH") };
        std::filesystem::path backendName  { getenvString("LOUVRE_INPUT_BACKEND") };

        bool usingEnvs { !backendsPath.empty() || !backendName.empty() };

        if (backendsPath.empty())
            backendsPath = LOUVRE_DEFAULT_BACKENDS_PATH;

        if (backendName.empty())
            backendName = LOUVRE_DEFAULT_INPUT_BACKEND;

        std::filesystem::path backendPathName = { backendsPath / "input" / backendName};
        backendPathName += ".so";

        retry:

        if (loadInputBackend(backendPathName))
        {
            if (!inputBackend->backendInitialize())
            {
                dlclose(inputBackendHandle);
                inputBackendHandle = nullptr;
                inputBackend = nullptr;
                LLog::error("[LCompositorPrivate::initInputBackend] Failed to initialize %s backend.", backendName.c_str());

                if (usingEnvs)
                {
                    usingEnvs = false;
                    backendPathName = std::filesystem::path(LOUVRE_DEFAULT_BACKENDS_PATH) / "input" / LOUVRE_DEFAULT_INPUT_BACKEND;
                    backendPathName += ".so";
                    goto retry;
                }

                return false;
            }
        }
        else
        {
            LLog::error("[LCompositorPrivate::initInputBackend] Failed to load %s backend.", backendPathName.c_str());

            if (usingEnvs)
            {
                usingEnvs = false;
                backendPathName = std::filesystem::path(LOUVRE_DEFAULT_BACKENDS_PATH) / "input" / LOUVRE_DEFAULT_INPUT_BACKEND;
                backendPathName += ".so";
                goto retry;
            }

            return false;
        }
    }

    LLog::debug("[LCompositorPrivate::initInputBackend] Input backend initialized successfully.");
    isInputBackendInitialized = true;
    return true;
}

void LCompositor::LCompositorPrivate::unitInputBackend(bool closeLib)
{
    if (inputBackend && isInputBackendInitialized)
    {
        inputBackend->backendUninitialize();
        LLog::debug("[LCompositorPrivate::unitInputBackend] Input backend uninitialized successfully.");
    }

    isInputBackendInitialized = false;

    if (closeLib)
    {
        if (inputBackendHandle)
            dlclose(inputBackendHandle);

        inputBackendHandle = nullptr;
        inputBackend = nullptr;
    }
}

void LCompositor::LCompositorPrivate::unitGraphicBackend(bool closeLib)
{
    if (painter)
    {
        delete painter;
        painter = nullptr;
    }

    if (isGraphicBackendInitialized && graphicBackend)
    {
        graphicBackend->backendUninitialize();
        LLog::debug("[LCompositorPrivate::unitGraphicBackend] Graphic backend uninitialized successfully.");
    }

    mainEGLDisplay = EGL_NO_DISPLAY;
    mainEGLContext = EGL_NO_CONTEXT;

    eglMakeCurrent(EGL_NO_DISPLAY,
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    isGraphicBackendInitialized = false;

    if (closeLib)
    {
        if (graphicBackendHandle)
            dlclose(graphicBackendHandle);

        graphicBackendHandle = nullptr;
        graphicBackend = nullptr;
    }
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
        // Notify first

        if (seat->keyboard())
            LCompositor::compositor()->destroyKeyboardRequest(seat->keyboard());

        if (seat->pointer())
            LCompositor::compositor()->destroyPointerRequest(seat->pointer());

        if (seat->touch())
            LCompositor::compositor()->destroyTouchRequest(seat->touch());

        if (seat->dnd())
            LCompositor::compositor()->destroyDNDRequest(seat->dnd());

        if (seat->clipboard())
            LCompositor::compositor()->destroyClipboardRequest(seat->clipboard());

        LCompositor::compositor()->destroySeatRequest(seat);

        // Then destroy

        if (seat->keyboard())
        {
            delete seat->m_keyboard;
            seat->m_keyboard = nullptr;
            LLog::debug("[LCompositorPrivate::unitSeat] Keyboard uninitialized successfully.");
        }

        if (seat->pointer())
        {
            delete seat->m_pointer;
            seat->m_pointer = nullptr;
            LLog::debug("[LCompositorPrivate::unitSeat] Pointer uninitialized successfully.");
        }

        if (seat->touch())
        {
            delete seat->m_touch;
            seat->m_touch = nullptr;
            LLog::debug("[LCompositorPrivate::unitSeat] Touch uninitialized successfully.");
        }

        if (seat->dnd())
        {
            delete seat->m_dnd;
            seat->m_dnd = nullptr;
            LLog::debug("[LCompositorPrivate::unitSeat] DND Manager uninitialized successfully.");
        }

        if (seat->clipboard())
        {
            delete seat->m_clipboard;
            seat->m_clipboard = nullptr;
            LLog::debug("[LCompositorPrivate::unitSeat] Clipboard Manager uninitialized successfully.");
        }

        delete seat;
        seat = nullptr;

        LLog::debug("[LCompositorPrivate::unitSeat] Seat uninitialized successfully.");
    }
}

bool LCompositor::LCompositorPrivate::loadGraphicBackend(const std::filesystem::path &path)
{
    if (graphicBackendHandle)
        dlclose(graphicBackendHandle);

    graphicBackendHandle = dlopen(path.c_str(), RTLD_LAZY);

    if (!graphicBackendHandle)
    {
        LLog::warning("[LCompositorPrivate::loadGraphicBackend] No graphic backend found at (%s)", path.c_str());
        return false;
    }

    LGraphicBackendInterface *(*getAPI)() = (LGraphicBackendInterface *(*)())dlsym(graphicBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::error("[LCompositorPrivate::loadGraphicBackend] Failed to load graphic backend (%s)", path.c_str());
        dlclose(graphicBackendHandle);
        return false;
    }

    graphicBackend = getAPI();

    if (graphicBackend)
        LLog::debug("[LCompositorPrivate::loadGraphicBackend] Graphic backend loaded successfully (%s).", path.c_str());

    return true;
}

bool LCompositor::LCompositorPrivate::loadInputBackend(const std::filesystem::path &path)
{
    if (inputBackendHandle)
        dlclose(inputBackendHandle);

    inputBackendHandle = dlopen(path.c_str(), RTLD_LAZY);

    if (!inputBackendHandle)
    {
        LLog::warning("[LCompositorPrivate::loadInputBackend] No input backend found at (%s).", path.c_str());
        return false;
    }

    LInputBackendInterface *(*getAPI)() = (LInputBackendInterface *(*)())dlsym(inputBackendHandle, "getAPI");

    if (!getAPI)
    {
        LLog::warning("[LCompositorPrivate::loadInputBackend] Failed to load input backend (%s).", path.c_str());
        dlclose(inputBackendHandle);
        return false;
    }

    inputBackend = getAPI();

    if (inputBackend)
        LLog::debug("[LCompositorPrivate::loadInputBackend] Input backend loaded successfully (%s).", path.c_str());

    return true;
}

void LCompositor::LCompositorPrivate::insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert)
{
    if (surfaceToInsert->prevSurface() == prevSurface)
        return;

    surfaces.erase(surfaceToInsert->imp()->compositorLink);

    if (prevSurface == surfaces.back())
    {
        surfaces.push_back(surfaceToInsert);
        surfaceToInsert->imp()->compositorLink = std::prev(surfaces.end());
    }
    else
        surfaceToInsert->imp()->compositorLink = surfaces.insert(std::next(prevSurface->imp()->compositorLink), surfaceToInsert);

    surfacesListChanged = true;
    surfaceToInsert->orderChanged();
}

void LCompositor::LCompositorPrivate::insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert)
{
    if (surfaceToInsert->nextSurface() == nextSurface)
        return;

    surfaces.erase(surfaceToInsert->imp()->compositorLink);
    surfaceToInsert->imp()->compositorLink = surfaces.insert(nextSurface->imp()->compositorLink, surfaceToInsert);
    surfacesListChanged = true;
    surfaceToInsert->orderChanged();
}

bool LCompositor::LCompositorPrivate::runningAnimations()
{
    bool running = false;

    for (LAnimation *anim : animations)
    {
        anim->imp()->processed = false;

        if (anim->imp()->running || anim->imp()->pendingDestroy)
            running = true;
    }

    return running;
}

void LCompositor::LCompositorPrivate::processAnimations()
{
    Int64 elapsed;
    retry:
    animationsVectorChanged = false;
    for (LAnimation *a : animations)
    {
        if (a->imp()->processed)
            continue;

        if (a->imp()->pendingDestroy)
        {
            delete a;
            goto retry;
        }

        a->imp()->processed = true;

        if (!a->imp()->running)
            continue;

        elapsed = (Int64)LTime::ms() - (Int64)a->imp()->beginTime;

        if (elapsed > (Int64)a->imp()->duration)
            elapsed = a->imp()->duration;

        a->imp()->value = (Float32)elapsed/(Float32)a->imp()->duration;

        if (a->imp()->onUpdate)
        {
            a->imp()->onUpdate(a);

            if (animationsVectorChanged)
                goto retry;
        }

        if (a->imp()->value == 1.f)
        {
            a->stop();

            if (animationsVectorChanged)
                goto retry;
        }

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
    renderMutex.lock();
}

void LCompositor::LCompositorPrivate::unlock()
{
    renderMutex.unlock();
}

void LCompositor::LCompositorPrivate::unlockPoll()
{
    if (pollUnlocked)
        return;

    pollUnlocked = true;
    uint64_t event_value = 1;
    ssize_t n = write(events[0].data.fd, &event_value, sizeof(event_value));
    L_UNUSED(n);
}

LPainter *LCompositor::LCompositorPrivate::findPainter()
{
    LPainter *painter = nullptr;
    std::thread::id threadId = std::this_thread::get_id();

    if (threadId == LCompositor::compositor()->mainThreadId())
        painter = LCompositor::compositor()->imp()->painter;
    else
    {
        for (LOutput *o : LCompositor::compositor()->outputs())
        {
            if (o->state() == LOutput::Initialized && o->imp()->threadId == threadId)
            {
                painter = o->painter();
                break;
            }
        }
    }

    return painter;
}

void LCompositor::LCompositorPrivate::sendPendingToplevelsConfiguration()
{
    for (LSurface *s : surfaces)
        if (s->toplevel())
            s->toplevel()->imp()->sendConfiguration();
}

void  LCompositor::LCompositorPrivate::sendPresentationTime()
{
    for (LOutput *o : outputs)
    {
        o->imp()->pageflipMutex.lock();
        if (o->imp()->stateFlags.check(LOutput::LOutputPrivate::HasUnhandledPresentationTime))
            for (LSurface *s : surfaces)
                s->imp()->sendPresentationFeedback(o);
        o->imp()->pageflipMutex.unlock();
    }
}
