#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLease.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>

#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LLockGuard.h>

#include <CZ/Louvre/Backends/LBackend.h>

#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Events/LSurfaceUnlockEvent.h>
#include <CZ/Louvre/Seat/LIdleListener.h>
#include <CZ/Louvre/Louvre.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RResourceTracker.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Core/CZCore.h>
#include <CZ/Core/CZTime.h>

#include <sys/eventfd.h>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>

using namespace CZ::Protocols::Wayland;

static LCompositor *s_compositor { nullptr };

LCompositor *CZ::compositor() noexcept
{
    return s_compositor;
}

LSeat *CZ::seat() noexcept
{
    return s_compositor->imp()->seat;
}

LCursor *CZ::cursor() noexcept
{
    return s_compositor->imp()->cursor.get();
}

LSessionLockManager *CZ::sessionLockManager() noexcept
{
    return s_compositor->imp()->sessionLockManager;
}

LActivationTokenManager *CZ::activationTokenManager() noexcept
{
    return s_compositor->imp()->activationTokenManager;
}

void LCompositor::removeGlobal(LGlobal *global) noexcept
{
    if (global->m_removed)
        return;

    global->m_removed = true;
    wl_global_remove(global->m_global);
}

bool LCompositor::addPosixSignal(int signal) noexcept
{
    if (state() == CompositorState::Uninitialized)
    {
        LLog(CZError, CZLN, "Failed to add posix signal {}. Signals must be added after the compositor is initialized", signal);
        return false;
    }

    if (posixSignals().contains(signal))
    {
        LLog(CZError, CZLN, "Failed to add posix signal {}. Already added", signal);
        return false;
    }

    imp()->posixSignalsChanged = true;
    imp()->posixSignals.emplace(signal);

    for (auto &threadData : imp()->threadsMap)
    {
        if (threadData.first != mainThreadId())
        {
            threadData.second.posixSignalsToDisable.emplace(signal);
            threadData.second.output->repaint();
        }
    }

    if (std::this_thread::get_id() == mainThreadId())
        imp()->handlePosixSignalChanges();
    else
        imp()->unlockPoll();

    return true;
}

bool LCompositor::removePosixSignal(int signal) noexcept
{
    if (state() == CompositorState::Uninitialized || !posixSignals().contains(signal))
        return false;

    if (state() == CompositorState::Uninitialized)
    {
        LLog(CZError, CZLN, "Failed to remove posix signal {}. Signals must be removed while the compositor is initialized.", signal);
        return false;
    }

    if (!posixSignals().contains(signal))
    {
        LLog(CZError, CZLN, "Attempt to remove unregistered posix signal {}", signal);
        return false;
    }

    imp()->posixSignalsChanged = true;
    imp()->posixSignals.erase(signal);

    if (std::this_thread::get_id() == mainThreadId())
        imp()->handlePosixSignalChanges();
    else
        imp()->unlockPoll();

    return true;
}

const std::unordered_set<int> &LCompositor::posixSignals() const noexcept
{
    return imp()->posixSignals;
}

LOutput *LCompositor::mostIntersectedOutput(const SkIRect &rect, bool initializedOnly) const noexcept
{
    LOutput *bestOutput { nullptr };
    Int32 bestArea { -1 };
    Int32 area;
    const auto &outputsVector { initializedOnly ? outputs() : seat()->outputs() };
    SkIRect test;

    for (auto *output : outputsVector)
    {
        test = rect;

        if (test.intersect(output->rect()))
            area = test.width() * test.height();
        else
            area = 0;

        if (area > bestArea)
        {
            bestOutput = output;
            bestArea = area;
        }
    }

    return bestOutput;
}

LCompositor::LCompositor() noexcept : LPRIVATE_INIT_UNIQUE(LCompositor)
{
    if (!s_compositor)
        s_compositor = this;

    LLog(CZInfo, CZLN, "Compositor created");
    imp()->core = CZCore::Get();
    imp()->core->m_owner = CZCore::Owner::Louvre;
}

LCompositor::~LCompositor() noexcept
{
    LLog(CZInfo, CZLN, "Compositor destroyed");
}

bool LCompositor::setBackend(std::shared_ptr<LBackend> backend) noexcept
{
    if (state() != LCompositor::Uninitialized)
        return false;

    imp()->backend = backend;
    return true;
}

LCompositor::CompositorState LCompositor::state() const noexcept
{
    return imp()->state;
}

LBackend *LCompositor::backend() const noexcept
{
    return imp()->backend.get();
}

bool LCompositor::event(const CZEvent &e) noexcept
{
    if (e.type() == CZEvent::Type::LSurfaceUnlock)
    {
        const auto &event { static_cast<const LSurfaceUnlockEvent&>(e) };

        if (event.surface)
            event.surface->imp()->unlockCommit(event.commitId);

        return true;
    }

    return CZObject::event(e);
}

bool LCompositor::start()
{
    if (compositor() != this)
    {
        LLog(CZError, CZLN, "Compositor already running");
        return false;
    }

    s_compositor = this;

    if (state() != CompositorState::Uninitialized)
    {
        LLog(CZError, CZLN, "Attempt to start a compositor already running. Ignoring it...");
        return false;
    }

    imp()->threadId = std::this_thread::get_id();
    imp()->state = CompositorState::Initializing;
    imp()->ream.reset();

    if (!imp()->initWayland())
    {
        LLog(CZFatal, CZLN, "Failed to initialize wl_display");
        goto fail;
    }

    if (!imp()->initSeat())
    {
        LLog(CZFatal, CZLN, "Failed to initialize seat");
        goto fail;
    }

    if (!imp()->initBackend())
    {
        LLog(CZFatal, CZLN, "Failed to initialize backend");
        goto fail;
    }

    if (!compositor()->createGlobalsRequest())
    {
        LLog(CZFatal, CZLN, "Failed to create globals");
        goto fail;
    }

    imp()->ream = RCore::Get();
    imp()->state = CompositorState::Initialized;
    initialized();
    return true;

    fail:
    imp()->unitCompositor();
    return false;
}

int LCompositor::dispatch(int msTimeout)
{
    if (state() == CompositorState::Uninitialized)
        return 0;

    pollfd pollfd {
        .fd = imp()->core->fd(),
        .events = POLLIN,
        .revents = 0
    };

    auto ream { RCore::Get() };
    const int ret { poll(&pollfd, 1, msTimeout) };
    const auto lock { LLockGuard() };

    seat()->setIsUserIdleHint(true);
    imp()->dispatchPresentationTimeEvents();
    imp()->handleUnreleasedBuffers();
    imp()->processRemovedGlobals();
    imp()->handlePosixSignalChanges();

    if (ret == 1)
        imp()->core->dispatch(0);

    if (seat()->enabled())
        if (!seat()->isUserIdleHint())
            for (const auto *idleListener : seat()->idleListeners())
                idleListener->resetTimer();

    cursor()->update();
    flushClients();
    imp()->handleDestroyedClients();

    if (ream)
        ream->clearGarbage();

    if (state() == CompositorState::Uninitializing)
    {
        uninitialized();
        imp()->unitCompositor();
    }
    else
    {
        // Handle tty switch
        if (seat()->imp()->ttyNumber != -1)
        {
            compositor()->imp()->unlockPoll();
            if (libseat_switch_session(seat()->libseatHandle(), seat()->imp()->ttyNumber) == 0)
                seat()->imp()->dispatchSeat();

            seat()->imp()->ttyNumber = -1;
        }
    }

    return 1;
}

int LCompositor::fd() const noexcept
{
    return imp()->core->fd();
}

void LCompositor::finish() noexcept
{
    if (state() == CompositorState::Uninitialized)
        return;

    imp()->state = CompositorState::Uninitializing;
    imp()->unlockPoll();
}

wl_display *LCompositor::display() noexcept
{
    return compositor()->imp()->display;
}

LCursor *LCompositor::cursor() const noexcept
{
    return imp()->cursor.get();
}

LSeat *LCompositor::seat() const noexcept
{
    return imp()->seat;
}

LSessionLockManager *LCompositor::sessionLockManager() const noexcept
{
    return imp()->sessionLockManager;
}

LActivationTokenManager *LCompositor::activationTokenManager() const noexcept
{
    return imp()->activationTokenManager;
}

void LCompositor::repaintAllOutputs() noexcept
{
    for (LOutput *o : imp()->outputs)
        o->repaint();
}

bool LCompositor::addOutput(LOutput *output)
{
    // Check if already initialized
    for (LOutput *o : outputs())
        if (o == output)
            return true;

    if (output->lease())
        output->lease()->finished();

    imp()->outputs.push_back(output);

    if (imp()->outputs.size() == 1)
        cursor()->setOutput(output);

    bool ok;
    output->imp()->state = LOutput::PendingInitialize;
    output->imp()->initPromise = std::promise<bool>();
    auto future { output->imp()->initPromise.get_future() };

    const auto didUnlock { LLockGuard::Unlock() };

    try {
        if (!output->backend()->init())
            output->imp()->initPromise.set_value(false);
        ok = future.get();
    }
    catch(...) {
        ok = false;
    }

    if (didUnlock)
        LLockGuard::Lock();

    if (!ok)
    {
        output->imp()->state = LOutput::Uninitialized;
        LLog(CZError, CZLN, "Failed to initialize output {}", output->name());
        CZVectorUtils::RemoveOne(imp()->outputs, output);

        if (imp()->outputs.empty())
            cursor()->setOutput(nullptr);

        return false;
    }

    for (auto *head : output->imp()->wlrOutputHeads)
        head->enabled(true);

    return true;
}

void LCompositor::removeOutput(LOutput *output)
{
    if (!backend())
        return;

    // Loop to check if output was added (initialized)
    for (LOutput *o : imp()->outputs)
    {
        if (o != output)
            continue;

        // Uninitializing outputs from their own thread is not allowed
        if (output->threadId() == std::this_thread::get_id())
            return;

        output->imp()->state = LOutput::PendingUninitialize;
        output->imp()->unitPromise = std::promise<bool>();
        auto future { output->imp()->unitPromise.get_future() };
        const auto didUnlock { LLockGuard::Unlock() };
        output->backend()->unit();
        assert(future.get());

        if (didUnlock)
            LLockGuard::Lock();

        output->imp()->state = LOutput::Uninitialized;

        for (auto *head : output->imp()->wlrOutputHeads)
            head->enabled(false);

        for (LSurface *s : surfaces())
            s->sendOutputLeaveEvent(output);

        CZVectorUtils::RemoveOne(imp()->outputs, output);

        // Remove all wl_outputs from clients
        for (LClient *client : clients())
        {
            for (std::size_t i = 0; i < client->imp()->outputGlobals.size();)
            {
                auto *global {client->imp()->outputGlobals[i]};

                if (output == global->output())
                {
                    global->m_output.reset();
                    client->imp()->outputGlobals[i] = std::move(client->imp()->outputGlobals.back());
                    client->imp()->outputGlobals.pop_back();
                    continue;
                }

                i++;
            }
        }

        if (output->imp()->global)
            removeGlobal(output->imp()->global);

        cursor()->m_intersectedOutputs.erase(output);

        if (cursor()->m_output == output)
            cursor()->m_output = nullptr;

        cursor()->move(1.f, 1.f);
        return;

    }
}

const std::list<LSurface *> &LCompositor::surfaces() const noexcept
{
    return imp()->surfaces;
}

const std::array<std::list<LSurface *>, 5> &LCompositor::layers() const noexcept
{
    return imp()->layers;
}

const std::vector<LOutput *> &LCompositor::outputs() const noexcept
{
    return imp()->outputs;
}

const std::vector<LClient *> &LCompositor::clients() const noexcept
{
    return imp()->clients;
}

void LCompositor::flushClients() noexcept
{
    compositor()->imp()->sendPendingConfigurations();
    wl_display_flush_clients(LCompositor::display());
}

LClient *LCompositor::getClientFromNativeResource(const wl_client *client) noexcept
{
    for (LClient *c : clients())
        if (c->client() == client)
            return c;
    return nullptr;
}

std::thread::id LCompositor::mainThreadId() const noexcept
{
    return imp()->threadId;
}

LGlobal *LCompositor::globalCreate(const wl_interface *interface, Int32 version, void *data, wl_global_bind_func_t bind)
{
    imp()->globals.emplace_back(new LGlobal(wl_global_create(display(), interface, version, nullptr, bind)));
    wl_global_set_user_data(const_cast<wl_global*>(imp()->globals.back()->global()), imp()->globals.back());
    imp()->globals.back()->userData = data;
    return imp()->globals.back();
}
