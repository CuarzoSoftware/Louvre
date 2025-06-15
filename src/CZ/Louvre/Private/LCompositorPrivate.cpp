#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/DRMLease/GDRMLeaseDevice.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>
#include <CZ/Louvre/Private/LCursorPrivate.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
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
#include <csignal>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <CZ/Louvre/Private/LFactory.h>

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
    eventLoop = wl_display_get_event_loop(display);
    eventLoopFd = wl_event_loop_get_fd(eventLoop);

    // This is used to force unlocking the main loop with imp()->unlockPoll()
    eventFd = eventfd(0, EFD_NONBLOCK);

    eventFdEventSource = wl_event_loop_add_fd(
        LCompositor::eventLoop(),
        eventFd,
        WL_EVENT_READABLE,
        [](int, unsigned int, void *) -> int {
            UInt64 value;
            auto ret = read(compositor()->imp()->eventFd, &value, sizeof(value));
            L_UNUSED(ret)
            compositor()->imp()->pollUnlocked = false;
            return 0;
        },
        nullptr);

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
    if (display)
    {        
        if (eventFdEventSource)
        {
            wl_event_source_remove(eventFdEventSource);
            eventFdEventSource = nullptr;
            close(eventFd);
            eventFd = -1;
        }

        while (!globals.empty())
        {
            delete globals.back();
            globals.pop_back();
        }
        wl_display_destroy(display);
        display = nullptr;
        eventLoop = nullptr; // Destroyed by wl_display_destroy
        eventLoopFd = -1;
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

    painter.reset(&initThreadData().painter);
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
    unitThreadData();

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


void LCompositor::LCompositorPrivate::insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert, CZBitset<InsertOptions> options)
{
    if (options.has(UpdateLayers))
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

    if (options.has(UpdateSurfaces) && surfaceToInsert->prevSurface() != prevSurface)
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

void LCompositor::LCompositorPrivate::insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert, CZBitset<InsertOptions> options)
{
    if (options.has(UpdateLayers))
    {
        assert(nextSurface->layer() == surfaceToInsert->layer() && "Surfaces do not belong to the same layer.");
        auto &layer { layers[nextSurface->layer()] };
        layer.erase(surfaceToInsert->imp()->layerLink);
        surfaceToInsert->imp()->layerLink = layer.insert(nextSurface->imp()->layerLink, surfaceToInsert);
    }

    if (options.has(UpdateSurfaces) && surfaceToInsert->nextSurface() != nextSurface)
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
    std::thread::id threadId { std::this_thread::get_id() };

    if (id != nullptr)
        threadId = *id;

    auto it { threadsMap.find(threadId) };

    if (it == threadsMap.end())
        return;

    while (!it->second.renderBuffersToDestroy.empty())
    {
        glDeleteFramebuffers(1, &it->second.renderBuffersToDestroy.back().framebufferId);
        it->second.renderBuffersToDestroy.pop_back();
    }
}

void LCompositor::LCompositorPrivate::addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::ThreadData &data)
{
    auto it { threadsMap.find(thread) };

    if (it == threadsMap.end())
        return;

    it->second.renderBuffersToDestroy.emplace_back(data);
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
    const uint64_t value { 1 };
    auto ret = write(eventFd, &value, sizeof(value));
    L_UNUSED(ret)
}

LPainter *LCompositor::LCompositorPrivate::findPainter()
{
    auto it { compositor()->imp()->threadsMap.find(std::this_thread::get_id()) };

    if (it != compositor()->imp()->threadsMap.end())
        return &it->second.painter;

    return nullptr;
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
        if (o->imp()->stateFlags.has(LOutput::LOutputPrivate::HasUnhandledPresentationTime))
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
        (*destroyedClients.begin())->destroy();
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
            gpu->m_leaseGlobal.reset(compositor()->createGlobal<Protocols::DRMLease::GDRMLeaseDevice>(0, gpu));
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

LCompositor::LCompositorPrivate::ThreadData &LCompositor::LCompositorPrivate::initThreadData(LOutput *output) noexcept
{
    return threadsMap.emplace(std::this_thread::get_id(), output).first->second;
}

void LCompositor::LCompositorPrivate::unitThreadData() noexcept
{
    threadsMap.erase(std::this_thread::get_id());
}

LCompositor::LCompositorPrivate::ThreadData::ThreadData(LOutput *output) noexcept : output(output), painter(output)
{
    LLog::debug("[LCompositorPrivate::ThreadData] Data created for thread %zu.", std::hash<std::thread::id>{}(std::this_thread::get_id()));

    // If not the main thread, disable posix signals right away
    if (std::this_thread::get_id() != compositor()->mainThreadId())
        posixSignalsToDisable = compositor()->imp()->posixSignals;
}

LCompositor::LCompositorPrivate::ThreadData::~ThreadData()
{
    LLog::debug("[LCompositorPrivate::ThreadData] Thread %zu data destroyed.", std::hash<std::thread::id>{}(std::this_thread::get_id()));
}

void LCompositor::LCompositorPrivate::disablePendingPosixSignals() noexcept
{
    auto it { threadsMap.find(std::this_thread::get_id()) };
    if (it == threadsMap.end())
        return;

    sigset_t sigset;

    while (!it->second.posixSignalsToDisable.empty())
    {
        LLog::debug("[LCompositorPrivate::disablePendingPosixSignals] Posix signal %d blocked in thread %zu.",
            *it->second.posixSignalsToDisable.begin(),
            std::hash<std::thread::id>{}(std::this_thread::get_id()));
        sigemptyset(&sigset);
        sigaddset(&sigset, *it->second.posixSignalsToDisable.begin());
        pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
        it->second.posixSignalsToDisable.erase(it->second.posixSignalsToDisable.begin());
    }
}

void LCompositor::LCompositorPrivate::unitPosixSignals() noexcept
{
    posixSignals.clear();
    posixSignalsChanged = true;
    handlePosixSignalChanges();
}

static int posixSignalHandler(int signal, void *)
{
    compositor()->onPosixSignal(signal);
    return 0;
}

void LCompositor::LCompositorPrivate::handlePosixSignalChanges() noexcept
{
    if (!posixSignalsChanged)
        return;

    posixSignalsChanged = false;

    auto signalsCopy { posixSignals };
    sigset_t sigset;

    // Handle destroyed signals
    for (auto sourceIt = posixSignalSources.begin(); sourceIt != posixSignalSources.end();)
    {
        // Still alive, nothing to do
        if (signalsCopy.contains(sourceIt->first))
        {
            // Only keep pending-to-add signals
            signalsCopy.erase(sourceIt->first);
            sourceIt++;
        }
        else // The user removed it
        {
            LLog::debug("[LCompositorPrivate::handlePosixSignalChanges] Event source removed for posix signal %d.", sourceIt->first);
            wl_event_source_remove(sourceIt->second);

            // libwayland doesn't unblock it automatically when removed
            sigemptyset(&sigset);
            sigaddset(&sigset, sourceIt->first);
            pthread_sigmask(SIG_UNBLOCK, &sigset, nullptr);

            sourceIt = posixSignalSources.erase(sourceIt);
        }
    }


    // Create added signals
    while (!signalsCopy.empty())
    {
        LLog::debug("[LCompositorPrivate::handlePosixSignalChanges] Event source created for posix signal %d.", *signalsCopy.begin());
        posixSignalSources[*signalsCopy.begin()] = wl_event_loop_add_signal(eventLoop, *signalsCopy.begin(), &posixSignalHandler, nullptr);
        signalsCopy.erase(signalsCopy.begin());
    }
}
