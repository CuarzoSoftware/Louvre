#include "RCore.h"
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/DRMLease/GDRMLeaseDevice.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Manager/LActivationTokenManager.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/Backends/DRM/LDRMBackend.h>
#include <CZ/Louvre/Backends/Wayland/LWaylandBackend.h>

#include <CZ/Ream/RDevice.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Core/CZCore.h>

#include <dlfcn.h>
#include <cassert>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>

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

static void clientDisconnectedEvent(wl_listener *listener, void *data)
{
    wl_client *client { (wl_client*)data };
    delete listener;

    LClient *disconnectedClient { compositor()->getClientFromNativeResource(client) };
    compositor()->onAnticipatedObjectDestruction(disconnectedClient);

    while (!disconnectedClient->imp()->resources.empty())
        disconnectedClient->imp()->resources.back()->destroy();

    CZVectorUtils::RemoveOneUnordered(compositor()->imp()->clients, disconnectedClient);
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
        LLog(CZError, CZLN, "wl_display_create failed");
        return false;
    }

    const char *socket { getenv("CZ_LOUVRE_WAYLAND_DISPLAY") };

    if (socket)
    {
        int socketFd { wl_display_add_socket(display, socket) };

        if (socketFd == -1)
        {
            LLog(CZError, CZLN, "Failed to add custom socket {}. Trying wl_display_add_socket_auto instead...", socket);
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
            LLog(CZError, CZLN, "Failed to add auto socket");
            return false;
        }
    }

    wl_display_init_shm(display);
    auto *eventLoop = wl_display_get_event_loop(display);

    waylandEventSource = CZEventSource::Make(wl_event_loop_get_fd(eventLoop), EPOLLIN, CZOwn::Borrow, [eventLoop](auto, auto) {
        wl_event_loop_dispatch(eventLoop, 0);
    });

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

    auto ream { RCore::Get() };

    while (!clients.empty())
        clients.back()->destroy();

    unitThreadData();
    unitBackend();
    unitSeat();
    unitPosixSignals();
    unitWayland();
    cursor.reset();
    core->dispatch();

    if (ream)
        ream->clearGarbage();

    LLog(CZInfo, CZLN, "Compositor uninitialized");
    state = CompositorState::Uninitialized;
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
            LLog(CZInfo, "LKeyboard uninitialized successfully");
        }

        if (seat->pointer())
        {
            delete seat->m_pointer;
            seat->m_pointer = nullptr;
            LLog(CZInfo, "LPointer uninitialized successfully");
        }

        if (seat->touch())
        {
            delete seat->m_touch;
            seat->m_touch = nullptr;
            LLog(CZInfo, "LTouch uninitialized successfully");
        }

        if (seat->dnd())
        {
            delete seat->m_dnd;
            seat->m_dnd = nullptr;
            LLog(CZInfo, "LDND uninitialized successfully");
        }

        if (seat->clipboard())
        {
            delete seat->m_clipboard;
            seat->m_clipboard = nullptr;
            LLog(CZInfo, "LClipboard Manager uninitialized successfully");
        }

        delete seat;
        seat = nullptr;

        LLog(CZInfo, "LSeat uninitialized successfully");
    }
}

void LCompositor::LCompositorPrivate::unlockPoll()
{
    core->unlockLoop();
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

void LCompositor::LCompositorPrivate::handleDestroyedClients()
{
    while (!destroyedClients.empty())
        (*destroyedClients.begin())->destroy();
}

void LCompositor::LCompositorPrivate::initDRMLeaseGlobals()
{
    auto ream { RCore::Get() };

    if (ream->platform() != RPlatform::DRM)
        return;

    for (auto *dev : ream->devices())
        if (dev->drmFd() >= 0 && dev->drmFdReadOnly() >= 0)
            dev->drmLeaseGlobal.reset(compositor()->createGlobal<Protocols::DRMLease::GDRMLeaseDevice>(0, dev));
}

void LCompositor::LCompositorPrivate::unitDRMLeaseGlobals()
{
    auto ream { RCore::Get() };

    // Not yet initialized
    if (!ream)
        return;

    for (auto *dev : ream->devices())
        if (dev->drmLeaseGlobal)
            compositor()->removeGlobal((LGlobal*)dev->drmLeaseGlobal.get());
}

LCompositor::LCompositorPrivate::ThreadData &LCompositor::LCompositorPrivate::initThreadData(LOutput *output) noexcept
{
    return threadsMap.emplace(std::this_thread::get_id(), output).first->second;
}

void LCompositor::LCompositorPrivate::unitThreadData() noexcept
{
    threadsMap.erase(std::this_thread::get_id());
}

LCompositor::LCompositorPrivate::ThreadData::ThreadData(LOutput *output) noexcept : output(output)
{
    LLog(CZDebug, CZLN, "Data created for thread {}", pthread_self());

    // If not the main thread, disable posix signals right away
    if (std::this_thread::get_id() != compositor()->mainThreadId())
        posixSignalsToDisable = compositor()->imp()->posixSignals;
}

LCompositor::LCompositorPrivate::ThreadData::~ThreadData()
{
    LLog(CZDebug, CZLN, "Thread {} data destroyed", pthread_self());
}

void LCompositor::LCompositorPrivate::disablePendingPosixSignals() noexcept
{
    auto it { threadsMap.find(std::this_thread::get_id()) };
    if (it == threadsMap.end())
        return;

    sigset_t sigset;

    while (!it->second.posixSignalsToDisable.empty())
    {
        LLog(CZDebug, CZLN, "Posix signal {} blocked in thread {}",
            *it->second.posixSignalsToDisable.begin(),
            pthread_self());
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
            LLog(CZDebug, CZLN, "Event source removed for posix signal {}", sourceIt->first);
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
        LLog(CZDebug, CZLN, "Event source created for posix signal {}", *signalsCopy.begin());
        posixSignalSources[*signalsCopy.begin()] = wl_event_loop_add_signal(wl_display_get_event_loop(display), *signalsCopy.begin(), &posixSignalHandler, nullptr);
        signalsCopy.erase(signalsCopy.begin());
    }
}

bool LCompositor::LCompositorPrivate::initBackend() noexcept
{
    if (!backend) // If exists, it was set by the user
    {
        if (getenv("WAYLAND_DISPLAY"))
            backend = std::shared_ptr<LWaylandBackend>(new LWaylandBackend());
        else
            backend = std::shared_ptr<LDRMBackend>(new LDRMBackend());
    }

    if (!backend->init())
    {
        backend.reset();
        return false;
    }

    // Assigns itself to LCompositorPrivate::cursor from the constructor
    new LCursor();
    initDRMLeaseGlobals();
    return true;
}

void LCompositor::LCompositorPrivate::unitBackend()
{
    if (!backend)
        return;

    while (!outputs.empty())
        compositor()->removeOutput(outputs.back());

    unitDRMLeaseGlobals();
    backend->unit();
    backend.reset();
}

void LCompositor::LCompositorPrivate::dispatchPresentationTimeEvents() noexcept
{
    std::lock_guard<std::mutex> lock { presentationMutex };
    for (auto *o : compositor()->outputs())
    {
        while (!o->imp()->presentationEventQueue.empty())
        {
            core->sendEvent(o->imp()->presentationEventQueue.front(), *o);
            o->imp()->presentationEventQueue.pop();
        }
    }
}

void LCompositor::LCompositorPrivate::handleUnreleasedBuffers() noexcept
{
    for (auto it = unreleasedBuffers.begin(); it != unreleasedBuffers.end();)
    {
        if (it->release())
            it = unreleasedBuffers.erase(it);
        else
            it++;
    }
}
