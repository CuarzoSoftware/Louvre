#include <protocols/DRMLease/RDRMLease.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCursorPrivate.h>
#include <protocols/Wayland/GOutput.h>

#include <LIdleListener.h>
#include <LAnimation.h>
#include <LNamespaces.h>
#include <LPopupRole.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDND.h>
#include <LView.h>
#include <LGlobal.h>
#include <LLog.h>
#include <LTime.h>
#include <LTimer.h>

#include <sys/eventfd.h>
#include <poll.h>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>

using namespace Louvre::Protocols::Wayland;

static LCompositor *s_compositor { nullptr };

LCompositor *Louvre::compositor() noexcept
{
    return s_compositor;
}

LSeat *Louvre::seat() noexcept
{
    return s_compositor->imp()->seat;
}

LCursor *Louvre::cursor() noexcept
{
    return s_compositor->imp()->cursor;
}

LSessionLockManager *Louvre::sessionLockManager() noexcept
{
    return s_compositor->imp()->sessionLockManager;
}

LActivationTokenManager *Louvre::activationTokenManager() noexcept
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

LOutput *LCompositor::mostIntersectedOutput(const LRect &rect, bool initializedOnly) const noexcept
{
    LOutput *bestOutput { nullptr };
    Int32 bestArea { -1 };
    const auto &outputsVector { initializedOnly ? outputs() : seat()->outputs() };
    LRect test;

    for (auto *output : outputsVector)
    {
        test = rect;
        test.clip(output->rect());

        if (test.area() > bestArea)
        {
            bestOutput = output;
            bestArea = test.area();
        }
    }

    return bestOutput;
}

LCompositor::LCompositor() noexcept : LPRIVATE_INIT_UNIQUE(LCompositor)
{
    if (!s_compositor)
        s_compositor = this;

    LLog::init();
    imp()->eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress ("eglBindWaylandDisplayWL");
    imp()->eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress ("eglQueryWaylandBufferWL");
    imp()->glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) eglGetProcAddress ("glEGLImageTargetRenderbufferStorageOES");
    imp()->glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress ("glEGLImageTargetTexture2DOES");

    imp()->defaultAssetsPath = LOUVRE_DEFAULT_ASSETS_PATH;
    imp()->defaultBackendsPath = LOUVRE_DEFAULT_BACKENDS_PATH;
    imp()->defaultGraphicBackendName = LOUVRE_DEFAULT_GRAPHIC_BACKEND;
    imp()->defaultInputBackendName = LOUVRE_DEFAULT_INPUT_BACKEND;

    imp()->version.major = LOUVRE_VERSION_MAJOR;
    imp()->version.minor = LOUVRE_VERSION_MINOR;
    imp()->version.patch = LOUVRE_VERSION_PATCH;
    imp()->version.build = LOUVRE_VERSION_BUILD;
}

LCompositor::~LCompositor() = default;

const LVersion &LCompositor::version() noexcept
{
    return s_compositor->imp()->version;
}

const std::filesystem::path &LCompositor::defaultBackendsPath() const noexcept
{
    return imp()->defaultBackendsPath;
}

const std::filesystem::path &LCompositor::defaultAssetsPath() const noexcept
{
    return imp()->defaultAssetsPath;
}

const std::string &LCompositor::defaultGraphicBackendName() const noexcept
{
    return imp()->defaultGraphicBackendName;
}

void *LCompositor::graphicBackendContextHandle() const noexcept
{
    return compositor()->imp()->graphicBackend->backendGetContextHandle();
}

UInt32 LCompositor::graphicBackendId() const noexcept
{
    return compositor()->imp()->graphicBackend->backendGetId();
}

void *LCompositor::inputBackendContextHandle() const noexcept
{
    return compositor()->imp()->inputBackend->backendGetContextHandle();
}

UInt32 LCompositor::inputBackendId() const noexcept
{
    return compositor()->imp()->inputBackend->backendGetId();
}

const std::string &LCompositor::defaultInputBackendName() const noexcept
{
    return imp()->defaultInputBackendName;
}

bool LCompositor::isGraphicBackendInitialized() const noexcept
{
    return imp()->isGraphicBackendInitialized;
}

bool LCompositor::isInputBackendInitialized() const noexcept
{
    return imp()->isInputBackendInitialized;
}

bool LCompositor::loadGraphicBackend(const std::filesystem::path &path) noexcept
{
    return imp()->loadGraphicBackend(path);
}

bool LCompositor::loadInputBackend(const std::filesystem::path &path) noexcept
{
    return imp()->loadInputBackend(path);
}

LCompositor::CompositorState LCompositor::state() const noexcept
{
    return imp()->state;
}

bool LCompositor::start()
{
    if (compositor() != this)
    {
        LLog::warning("[LCompositor::start] Compositor already running. Two Louvre compositors can not live in the same process.");
        return false;
    }

    s_compositor = this;

    if (state() != CompositorState::Uninitialized)
    {
        LLog::warning("[LCompositor::start] Attempting to start a compositor already running. Ignoring...");
        return false;
    }

    imp()->threadId = std::this_thread::get_id();
    imp()->state = CompositorState::Initializing;

    compositor()->imp()->epollFd = epoll_create1(EPOLL_CLOEXEC);
    compositor()->imp()->events[LEV_LIBSEAT].data.fd = -1;

    if (!imp()->initWayland())
    {
        LLog::fatal("[LCompositor::start] Failed to init Wayland.");
        goto fail;
    }

    if (!imp()->initSeat())
    {
        LLog::fatal("[LCompositor::start] Failed to init seat.");
        goto fail;
    }

    if (!imp()->initGraphicBackend())
    {
        LLog::fatal("[LCompositor::start] Failed to init graphic backend.");
        goto fail;
    }

    if (!imp()->initInputBackend())
    {
        LLog::fatal("[LCompositor::start] Failed to init input backend.");
        goto fail;
    }

    if (!compositor()->createGlobalsRequest())
    {
        LLog::fatal("[LCompositorPrivate::initWayland] Failed to create globals.");
        goto fail;
    }

    imp()->state = CompositorState::Initialized;
    initialized();

    imp()->events[LEV_UNLOCK].events = EPOLLIN;
    imp()->events[LEV_UNLOCK].data.fd = eventfd(0, EFD_NONBLOCK);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_UNLOCK].data.fd,
              &compositor()->imp()->events[LEV_UNLOCK]);

    return true;

    fail:
    imp()->unitCompositor();
    return false;
}

Int32 LCompositor::processLoop(Int32 msTimeout)
{
    if (state() == CompositorState::Uninitialized)
        return 0;

    if (!seat()->enabled())
        msTimeout = 100;

    epoll_event events[4];

    Int32 nEvents = epoll_wait(imp()->epollFd,
                         events,
                         4,
                         msTimeout);

    imp()->lock();
    seat()->setIsUserIdleHint(true);
    imp()->sendPresentationTime();
    imp()->processRemovedGlobals();

    /* In certain older libseat versions, a POLLIN event may not be generated
     * during session switching. To ensure stability, we always dispatch
     * events; otherwise, the compositor might crash when a user is in a different
     * session and a new DRM connector is plugged in. */

    seat()->imp()->dispatchSeat();

    /* Some input backends, such as Libinput, require periodic handling of events related to device plugging and unplugging,
     * even if the session is disabled. Failing to do so may lead to unexpected bugs and result in a compositor crash. */

    if (!seat()->enabled())
    {
        imp()->processAnimations();
        imp()->inputBackend->backendForceUpdate();
    }

    bool flush { false };

    for (Int32 i = 0; i < nEvents; i++)
    {
        // Backend + User
        if (events[i].data.fd == imp()->events[LEV_AUX].data.fd)
        {
            if (seat()->enabled())
            {
                wl_event_loop_dispatch(imp()->auxEventLoop, 0);
                flush = true;
            }
        }
        // Wayland
        else if (events[i].data.fd == imp()->events[LEV_WAYLAND].data.fd)
        {
            if (seat()->enabled())
            {
                wl_event_loop_dispatch(imp()->waylandEventLoop, 0);
                flush = true;
            }
        }
        // Event fd
        else if (events[i].data.fd == imp()->events[LEV_UNLOCK].data.fd)
        {
            UInt64 eventValue;
            ssize_t n = read(imp()->events[0].data.fd, &eventValue, sizeof(eventValue));
            L_UNUSED(n);
            imp()->pollUnlocked = false;
        }
        else if (events[i].data.fd == imp()->events[LEV_LIBSEAT].data.fd)
        {
            seat()->imp()->dispatchSeat();
        }
    }

    if (seat()->enabled())
    {
        if (!seat()->isUserIdleHint())
            for (const auto *idleListener : seat()->idleListeners())
                idleListener->resetTimer();

        if (flush)
        {
            cursor()->imp()->textureUpdate();
            flushClients();
        }

        imp()->destroyPendingRenderBuffers(nullptr);
        imp()->handleDestroyedClients();
    }

    if (state() == CompositorState::Uninitializing)
    {
        uninitialized();

        imp()->processAnimations();

        while (!outputs().empty())
            removeOutput(outputs().back());

        for (LTexture *texture : imp()->textures)
            texture->reset();

        imp()->unitInputBackend(true);

        if (imp()->cursor)
            delete imp()->cursor;

        imp()->unitGraphicBackend(true);
        imp()->unitSeat();
        imp()->unitWayland();

        retry:
        for (LAnimation *a : imp()->animations)
        {
            if (a->m_destroyOnFinish)
            {
                delete a;
                goto retry;
            }
            else
                a->stop();
        }

        while (!imp()->oneShotTimers.empty())
            delete imp()->oneShotTimers.back();

        imp()->state = CompositorState::Uninitialized;

        if (imp()->epollFd != -1)
            close(imp()->epollFd);
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

        imp()->unlock();
    }

    return 1;
}

Int32 LCompositor::fd() const noexcept
{
    return imp()->epollFd;
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

wl_event_source *LCompositor::addFdListener(int fd, void *userData, int (*callback)(int, unsigned int, void *), UInt32 flags)
{
    return wl_event_loop_add_fd(compositor()->imp()->auxEventLoop, fd, flags, callback, userData);
}

void LCompositor::removeFdListener(wl_event_source *source)
{
    wl_event_source_remove(source);
}

LCursor *LCompositor::cursor() const noexcept
{
    return imp()->cursor;
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
        cursor()->imp()->setOutput(output);

    if (!output->imp()->initialize())
    {
        LLog::error("[LCompositor::addOutput] Failed to initialize output %s.", output->name());
        LVectorRemoveOne(imp()->outputs, output);

        if (imp()->outputs.empty())
            cursor()->imp()->setOutput(nullptr);

        return false;
    }

    return true;
}

void LCompositor::removeOutput(LOutput *output)
{
    if (!isGraphicBackendInitialized())
        return;

    // Loop to check if output was added (initialized)
    for (LOutput *o : imp()->outputs)
    {
        // Was initialized
        if (o == output)
        {
            // Uninitializing outputs from their own thread is not allowed
            if (output->threadId() == std::this_thread::get_id())
                return;

            output->imp()->callLockACK.store(false);
            output->imp()->callLock.store(false);
            output->repaint();
            output->imp()->state = LOutput::PendingUninitialize;
            imp()->unlock();

            Int32 waitLimit = 0;

            while (!output->imp()->callLockACK.load() && waitLimit < 1000)
            {
                usleep(1000);
                waitLimit++;
            }

            imp()->lock();
            imp()->graphicBackend->outputUninitialize(output);

            while (output->imp()->state != LOutput::Uninitialized)
                usleep(1000);

            output->imp()->callLock.store(true);

            for (LSurface *s : surfaces())
                s->sendOutputLeaveEvent(output);

            for (LView *v : imp()->views)
                v->removeThread(o->threadId());

            LVectorRemoveOne(imp()->outputs, output);

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

            LVectorRemoveOneUnordered(cursor()->imp()->intersectedOutputs, output);

            if (cursor()->m_output == output)
                cursor()->m_output = nullptr;

            cursor()->move(1.f, 1.f);
            return;
        }
    }
}

const std::list<LSurface *> &LCompositor::surfaces() const noexcept
{
    return imp()->surfaces;
}

const std::list<LSurface *> &LCompositor::layer(LSurfaceLayer layer) const noexcept
{
    return imp()->layers[layer];
}

const std::vector<LOutput *> &LCompositor::outputs() const noexcept
{
    return imp()->outputs;
}

const std::vector<LClient *> &LCompositor::clients() const noexcept
{
    return imp()->clients;
}

EGLDisplay LCompositor::eglDisplay() noexcept
{
    return compositor()->imp()->mainEGLDisplay;
}

EGLContext LCompositor::eglContext() noexcept
{
    return compositor()->imp()->mainEGLContext;
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
    imp()->globals.back()->setUserData((UIntPtr)data);
    return imp()->globals.back();
}
