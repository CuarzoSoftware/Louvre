#include <protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <protocols/DRMLease/GDRMLeaseDevice.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LPopupRolePrivate.h>
#include <private/LFactory.h>
#include <LActivationTokenManager.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LBackgroundBlur.h>
#include <LAnimation.h>
#include <LClipboard.h>
#include <LKeyboard.h>
#include <LPointer.h>
#include <LOpenGL.h>
#include <LTouch.h>
#include <LGlobal.h>
#include <LGPU.h>
#include <LTime.h>
#include <LTimer.h>
#include <LLog.h>
#include <EGL/egl.h>
#include <dlfcn.h>
#include <string.h>
#include <cassert>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <private/LFactory.h>

void LCompositor::LCompositorPrivate::processRemovedGlobals()
{
    for (auto it = globals.begin(); it != globals.end();)
    {
        if ((*it)->m_removed)
        {
            if ((*it)->m_destroyRoundtrips == 3)
            {
                delete *it;
                it = globals.erase(it);
                continue;
            }
            else
                (*it)->m_destroyRoundtrips++;
        }

        it++;
    }
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
    wl_client *client { (wl_client*)data };

    LClient *disconnectedClient { compositor()->getClientFromNativeResource(client) };
    compositor()->onAnticipatedObjectDestruction(disconnectedClient);

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

    LVectorRemoveOneUnordered(compositor()->imp()->clients, disconnectedClient);
    delete disconnectedClient;
}

static void clientConnectedEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);
    wl_client *client { (wl_client*)data };
    LClient::Params *params { new LClient::Params };
    params->client = client;

    wl_listener *destroyListener { new wl_listener() };
    destroyListener->notify = clientDisconnectedEvent;
    wl_client_add_destroy_listener(client, destroyListener);

    // Append client to the compositor list
    compositor()->imp()->clients.push_back(
        LFactory::createObject<LClient>(params));
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

    wl_display_init_shm(display);
    waylandEventLoop = wl_display_get_event_loop(display);
    auxEventLoop = wl_event_loop_create();

    compositor()->imp()->events[LEV_WAYLAND].events = EPOLLIN | EPOLLOUT;
    compositor()->imp()->events[LEV_WAYLAND].data.fd = wl_event_loop_get_fd(waylandEventLoop);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_WAYLAND].data.fd,
              &compositor()->imp()->events[LEV_WAYLAND]);

    compositor()->imp()->events[LEV_AUX].events = EPOLLIN | EPOLLOUT;
    compositor()->imp()->events[LEV_AUX].data.fd = wl_event_loop_get_fd(auxEventLoop);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_AUX].data.fd,
              &compositor()->imp()->events[LEV_AUX]);

    // Listen for client connections
    clientConnectedListener.notify = &clientConnectedEvent;
    wl_display_add_client_created_listener(display, &clientConnectedListener);
    LFactory::createObject<LSessionLockManager>(&sessionLockManager);
    LFactory::createObject<LActivationTokenManager>(&activationTokenManager);
    wl_display_set_global_filter(display, [](const wl_client *client, const wl_global *global, void */*data*/) -> bool
    {
        LGlobal *lGlobal { nullptr };

        for (LGlobal *g : compositor()->imp()->globals)
        {
            if (g->global() == global)
            {
                lGlobal = g;
                break;
            }
        }

        // Always accept globals not created with LCompositor::createGlobal(), e.g. SHM, WL_DRM, etc
        if (!lGlobal)
            return true;

        LClient *lClient { compositor()->getClientFromNativeResource(client) };

        if (!lClient)
            return true;

        return compositor()->globalsFilter(lClient, lGlobal);
    }, nullptr);

    return true;
}

void LCompositor::LCompositorPrivate::unitWayland()
{
    if (auxEventLoop)
    {
        wl_event_loop_destroy(auxEventLoop);
        auxEventLoop = nullptr;
    }

    if (display)
    {
        while (!globals.empty())
        {
            delete globals.back();
            globals.pop_back();
        }
        wl_display_destroy(display);
        display = nullptr;
    }

    if (activationTokenManager)
    {
        compositor()->onAnticipatedObjectDestruction(activationTokenManager);
        delete activationTokenManager;
        activationTokenManager = nullptr;
    }

    if (sessionLockManager)
    {
        compositor()->onAnticipatedObjectDestruction(sessionLockManager);
        delete sessionLockManager;
        sessionLockManager = nullptr;
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
        std::filesystem::path backendName  { getenvString("LOUVRE_GRAPHIC_BACKEND")};

        bool usingEnvs = !backendsPath.empty() || !backendName.empty();

        if (backendsPath.empty())
            backendsPath = LOUVRE_DEFAULT_BACKENDS_PATH;

        if (backendName.empty())
        {
            if (getenv("WAYLAND_DISPLAY"))
                backendName = "wayland";
            else
                backendName = LOUVRE_DEFAULT_GRAPHIC_BACKEND;
        }

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

    char const *eglExts = eglQueryString(mainEGLDisplay, EGL_EXTENSIONS);

    WL_bind_wayland_display = LOpenGL::hasExtension(eglExts, "EGL_WL_bind_wayland_display");

    if (WL_bind_wayland_display)
        eglBindWaylandDisplayWL(eglDisplay(), display);

    painter = new LPainter();
    cursor = new LCursor();
    initDRMLeaseGlobals();
    initDMAFeedback();
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
        {
            if (getenv("WAYLAND_DISPLAY"))
                backendName = "wayland";
            else
                backendName = LOUVRE_DEFAULT_INPUT_BACKEND;
        }

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
    unitDMAFeedback();
    unitDRMLeaseGlobals();

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
    LFactory::createObject<LSeat>(&seat);
    return true;
}

void LCompositor::LCompositorPrivate::unitSeat()
{
    if (seat)
    {
        // Notify first

        if (seat->keyboard())
            compositor()->onAnticipatedObjectDestruction(seat->keyboard());

        if (seat->pointer())
            compositor()->onAnticipatedObjectDestruction(seat->pointer());

        if (seat->touch())
            compositor()->onAnticipatedObjectDestruction(seat->touch());

        if (seat->dnd())
            compositor()->onAnticipatedObjectDestruction(seat->dnd());

        if (seat->clipboard())
            compositor()->onAnticipatedObjectDestruction(seat->clipboard());

        compositor()->onAnticipatedObjectDestruction(seat);

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

void LCompositor::LCompositorPrivate::notifyOrderChangeFromSurface(LSurface *from)
{
    compositor()->imp()->surfaceRaiseAllowedCounter++;

    if (from)
        for (auto it = from->imp()->compositorLink; it != surfaces.end(); it++)
            (*it)->orderChanged();
    else
        for (LSurface *surface : surfaces)
            surface->orderChanged();

    compositor()->imp()->surfaceRaiseAllowedCounter--;
}


void LCompositor::LCompositorPrivate::insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert, LBitset<InsertOptions> options)
{
    if (options.check(UpdateLayers))
    {
        if (prevSurface)
        {
            assert(prevSurface->layer() == surfaceToInsert->layer() && "Surfaces do not belong to the same layer.");
            auto &layer { layers[prevSurface->layer()] };
            layer.erase(surfaceToInsert->imp()->layerLink);

            if (prevSurface == layer.back())
            {
                layer.emplace_back(surfaceToInsert);
                surfaceToInsert->imp()->layerLink = std::prev(layer.end());
            }
            else
                surfaceToInsert->imp()->layerLink = layer.insert(std::next(prevSurface->imp()->layerLink), surfaceToInsert);
        }
        else
        {
            auto &layer { layers[surfaceToInsert->layer()] };
            layer.erase(surfaceToInsert->imp()->layerLink);
            layer.emplace_front(surfaceToInsert);
            surfaceToInsert->imp()->layerLink = layer.begin();
        }
    }

    if (options.check(UpdateSurfaces) && surfaceToInsert->prevSurface() != prevSurface)
    {
        if (prevSurface)
        {
            surfaces.erase(surfaceToInsert->imp()->compositorLink);

            if (prevSurface == surfaces.back())
            {
                surfaces.push_back(surfaceToInsert);
                surfaceToInsert->imp()->compositorLink = std::prev(surfaces.end());
            }
            else
                surfaceToInsert->imp()->compositorLink = surfaces.insert(std::next(prevSurface->imp()->compositorLink), surfaceToInsert);
        }
        else
        {
            surfaces.erase(surfaceToInsert->imp()->compositorLink);
            surfaces.emplace_front(surfaceToInsert);
            surfaceToInsert->imp()->compositorLink = surfaces.begin();
            surfacesListChanged = true;
        }

        notifyOrderChangeFromSurface(prevSurface);
    }

#if LOUVRE_ASSERT_CHECKS == 1
    assertSurfacesOrder();
#endif
}

void LCompositor::LCompositorPrivate::insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert, LBitset<InsertOptions> options)
{
    if (options.check(UpdateLayers))
    {
        assert(nextSurface->layer() == surfaceToInsert->layer() && "Surfaces do not belong to the same layer.");
        auto &layer { layers[nextSurface->layer()] };
        layer.erase(surfaceToInsert->imp()->layerLink);
        surfaceToInsert->imp()->layerLink = layer.insert(nextSurface->imp()->layerLink, surfaceToInsert);
    }

    if (options.check(UpdateSurfaces) && surfaceToInsert->nextSurface() != nextSurface)
    {
        surfaces.erase(surfaceToInsert->imp()->compositorLink);
        surfaceToInsert->imp()->compositorLink = surfaces.insert(nextSurface->imp()->compositorLink, surfaceToInsert);
        surfacesListChanged = true;
        notifyOrderChangeFromSurface(surfaceToInsert);
    }

#if LOUVRE_ASSERT_CHECKS == 1
    assertSurfacesOrder();
#endif
}

bool LCompositor::LCompositorPrivate::runningAnimations()
{
    bool running { false };

    for (LAnimation *anim : animations)
    {
        anim->m_processed = false;

        if (anim->m_running || anim->m_pendingDestroy)
            running = true;
    }

    return running;
}

void LCompositor::LCompositorPrivate::processAnimations()
{
    Int64 elapsed;
    Int64 duration;
    retry:
    animationsVectorChanged = false;
    for (LAnimation *a : animations)
    {
        if (a->m_processed)
            continue;

        if (a->m_pendingDestroy)
        {
            delete a;
            goto retry;
        }

        a->m_processed = true;

        if (!a->m_running)
            continue;

        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - a->m_beginTime).count();

        duration = static_cast<Int64>(a->m_duration);

        if (elapsed >= duration)
            a->m_value = 1.0;
        else
            a->m_value = static_cast<Float64>(elapsed)/static_cast<Float64>(duration);

        if (a->m_onUpdate)
        {
            a->m_onUpdate(a);

            if (animationsVectorChanged)
                goto retry;
        }

        if (a->m_value == 1.0)
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
        glDeleteFramebuffers(1, &threadData.renderBuffersToDestroy.back().framebufferId);
        threadData.renderBuffersToDestroy.pop_back();
    }
}

void LCompositor::LCompositorPrivate::addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::ThreadData &data)
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

    if (threadId == compositor()->mainThreadId())
        painter = compositor()->imp()->painter;
    else
    {
        for (LOutput *o : compositor()->outputs())
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

void LCompositor::LCompositorPrivate::sendPendingConfigurations()
{
    for (LSurface *s : surfaces)
    {
        s->backgroundBlur()->sendPendingConfiguration();

        if (s->toplevel())
            s->toplevel()->sendPendingConfiguration();
        else if (s->popup())
            s->popup()->sendPendingConfiguration();
        else if (s->sessionLock())
            s->sessionLock()->sendPendingConfiguration();
    }

    for (LClient *c : clients)
        for (auto *g : c->wlrOutputManagerGlobals())
            g->done();
}

void LCompositor::LCompositorPrivate::sendPresentationTime()
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

void LCompositor::LCompositorPrivate::initDMAFeedback() noexcept
{
    if (graphicBackend->backendGetDMAFormats()->empty())
    {
        LLog::warning("[LCompositorPrivate::initDMAFeedback] DMA formats not available.");
        return;
    }

    char name[] = "/louvre-XXXXXXXX";
    Int32 i, rw;
    UInt8 *map, *ptr;
    UInt16 formatIndex { 0 };
retryName:
    i = 8;
    while (i < 15)
        name[i++] = 48 + ((LTime::ms() + rand()) % 10);
    name[i] = '\0';

    rw = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);

    if (rw < 0)
    {
        if (errno == EEXIST)
            goto retryName;
        else
            goto err;
    }

    dmaFeedback.tableFd = shm_open(name, O_RDONLY, 0);

    if (dmaFeedback.tableFd < 0)
    {
        shm_unlink(name);
        close(rw);
        goto err;
    }

    shm_unlink(name);

    if (fchmod(rw, 0) != 0) {
        close(rw);
        close(dmaFeedback.tableFd);
        goto err;
    }

    dmaFeedback.tableSize = 16 * graphicBackend->backendGetDMAFormats()->size();

    int ret;
    do {
        ret = ftruncate(rw, dmaFeedback.tableSize);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        close(rw);
        close(dmaFeedback.tableFd);
        goto err;
    }

    map = (UInt8*)mmap(NULL, dmaFeedback.tableSize, PROT_READ | PROT_WRITE, MAP_SHARED, rw, 0);

    if (map == MAP_FAILED)
    {
        close(rw);
        close(dmaFeedback.tableFd);
        goto err;
    }

    wl_array_init(&dmaFeedback.formatIndices);
    wl_array_init(&dmaFeedback.scanoutIndices);
    dmaFeedback.device = graphicBackend->backendGetAllocatorDevice()->dev();

    ptr = map;
    for (const auto &format : *graphicBackend->backendGetDMAFormats())
    {
        memcpy(ptr, &format.format, sizeof(UInt32));
        ptr+=8;
        memcpy(ptr, &format.modifier, sizeof(UInt64));
        ptr+=8;

        for (const auto &scanoutFmt : *graphicBackend->backendGetScanoutDMAFormats())
        {
            if (scanoutFmt == format)
            {
                *(UInt16*)wl_array_add(&dmaFeedback.scanoutIndices, sizeof(UInt16)) = formatIndex;
                break;
            }
        }

        // Repeating formats across different tranches of the same device is permitted
        // if they have different flags
        *(UInt16*)wl_array_add(&dmaFeedback.formatIndices, sizeof(UInt16)) = formatIndex;

        formatIndex++;
    }

    munmap(map, dmaFeedback.tableSize);
    close(rw);

    LLog::debug("[LCompositorPrivate::initDMAFeedback] DMA Feedback formats table created successfully.");
    return;
err:
    dmaFeedback.tableFd = -1;
    LLog::error("[LCompositorPrivate::initDMAFeedback] Failed to create DMA Feedback formats table: %s.", name);
}

void LCompositor::LCompositorPrivate::unitDMAFeedback() noexcept
{
    if (dmaFeedback.tableFd != -1)
    {
        close(dmaFeedback.tableFd);
        dmaFeedback.tableFd = -1;
        wl_array_release(&dmaFeedback.formatIndices);
        wl_array_release(&dmaFeedback.scanoutIndices);
    }
}

void LCompositor::LCompositorPrivate::handleDestroyedClients()
{
    while (!destroyedClients.empty())
        wl_client_destroy((*destroyedClients.begin())->client());
}

#if LOUVRE_ASSERT_CHECKS == 1
void LCompositor::LCompositorPrivate::assertSurfacesOrder()
{
    if (surfaces.empty())
        return;

    LSurface *surf { surfaces.front() };

    for (int i = 0; i <= LLayerOverlay; i++)
    {
        for (LSurface *ls : layers[i])
        {
            assert(ls == surf);
            surf = surf->nextSurface();
        }
    }

    LLog::log("assertSurfacesOrder() ok");
}
#endif

void LCompositor::LCompositorPrivate::initDRMLeaseGlobals()
{
    for (LGPU *gpu : *graphicBackend->backendGetDevices())
        if (gpu->fd() != -1 && gpu->roFd() != -1)
            gpu->m_leaseGlobal.reset(compositor()->createGlobal<Protocols::DRMLease::GDRMLeaseDevice>(gpu));
}

void LCompositor::LCompositorPrivate::unitDRMLeaseGlobals()
{
    // Not yet initialized
    if (!graphicBackend)
        return;

    for (LGPU *gpu : *graphicBackend->backendGetDevices())
        if (gpu->m_leaseGlobal)
            compositor()->removeGlobal(gpu->m_leaseGlobal);
}
