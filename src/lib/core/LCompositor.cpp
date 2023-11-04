#include <csignal>
#include <iostream>
#include <ostream>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LAnimationPrivate.h>
#include <private/LViewPrivate.h>

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
#include <sys/eventfd.h>

using namespace Louvre::Protocols::Wayland;

static LCompositor *s_compositor = nullptr;

LCompositor::LCompositor()
{
    if (!s_compositor)
        s_compositor = this;

    LLog::init();
    LSurface::LSurfacePrivate::getEGLFunctions();
    m_imp = new LCompositorPrivate();
    imp()->compositor = this;
}

LCompositor *LCompositor::compositor()
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
    if (compositor() != this)
    {
        LLog::warning("[LCompositor::start()] Compositor already running. Two Louvre compositors can not live in the same process.");
        return false;
    }

    s_compositor = this;

    if (state() != CompositorState::Uninitialized)
    {
        LLog::warning("[LCompositor::start()] Attempting to start a compositor already running. Ignoring...");
        return false;
    }

    imp()->threadId = std::this_thread::get_id();
    imp()->state = CompositorState::Initializing;

    compositor()->imp()->epollFd = epoll_create1(EPOLL_CLOEXEC);
    compositor()->imp()->events[1].data.fd = -1;

    if (!imp()->initWayland())
    {
        LLog::fatal("[LCompositor::start()] Failed to init Wayland.");
        goto fail;
    }

    if (!imp()->initSeat())
    {
        LLog::fatal("[LCompositor::start()] Failed to init seat.");
        goto fail;
    }

    if (!imp()->initGraphicBackend())
    {
        LLog::fatal("[LCompositor::start()] Failed to init graphic backend.");
        goto fail;
    }

    if (!imp()->initInputBackend())
    {
        LLog::fatal("[LCompositor::start()] Failed to init input backend.");
        goto fail;
    }

    seat()->initialized();

    imp()->state = CompositorState::Initialized;
    initialized();

    imp()->events[0].events = EPOLLIN;
    imp()->events[0].data.fd = eventfd(0, EFD_NONBLOCK);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[0].data.fd,
              &compositor()->imp()->events[0]);

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
        msTimeout = 1000;

    epoll_event events[3];

    Int32 nEvents = epoll_wait(imp()->epollFd,
                         events,
                         3,
                         msTimeout);

    imp()->lock();

    imp()->processRemovedGlobals();

    /* In certain older libseat versions, a POLLIN event may not be generated
     * during session switching. To ensure stability, we always dispatch
     * events; otherwise, the compositor might crash when a user is in a different
     * session and a new DRM connector is plugged in. */

    seat()->imp()->dispatchSeat();

    for (Int32 i = 0; i < nEvents; i++)
    {
        // Wayland
        if (events[i].data.fd == imp()->events[2].data.fd)
        {
            if (seat()->enabled())
            {
                wl_event_loop_dispatch(imp()->eventLoop, 0);
                flushClients();
                cursor()->imp()->textureUpdate();
            }
        }
        // Event fd
        else if (events[i].data.fd == imp()->events[0].data.fd)
        {
            UInt64 eventValue;
            ssize_t n = read(imp()->events[0].data.fd, &eventValue, sizeof(eventValue));
            L_UNUSED(n);
            imp()->pollUnlocked = false;
        }
        else if (events[i].data.fd == imp()->events[1].data.fd)
        {
            seat()->imp()->dispatchSeat();
        }
    }

    imp()->destroyPendingRenderBuffers(nullptr);
    imp()->destroyNativeTextures(imp()->nativeTexturesToDestroy);

    if (state() == CompositorState::Uninitializing)
    {
        uninitialized();

        imp()->processAnimations();

        while (!outputs().empty())
            removeOutput(outputs().back());

        imp()->unitInputBackend(true);

        while (!imp()->textures.empty())
            delete imp()->textures.back();

        imp()->unitGraphicBackend(true);
        imp()->unitSeat();

        imp()->unitWayland();

        if (imp()->cursor)
        {
            delete imp()->cursor;
            imp()->cursor = nullptr;
        }

        while (!imp()->animations.empty())
        {
            delete imp()->animations.back();
            imp()->animations.pop_back();
        }

        imp()->state = CompositorState::Uninitialized;

        if (imp()->epollFd != -1)
            close(imp()->epollFd);
    }
    else
        imp()->unlock();

    return 1;
}

Int32 LCompositor::fd() const
{
    return imp()->epollFd;
}

void LCompositor::finish()
{
    if (state() == CompositorState::Uninitialized)
        return;

    imp()->state = CompositorState::Uninitializing;
    imp()->unlockPoll();
}

void LCompositor::LCompositorPrivate::raiseChildren(LSurface *surface)
{
    if (surface->nextSurface())
    {
        surfaces.erase(surface->imp()->compositorLink);
        surfaces.push_back(surface);
        surface->imp()->compositorLink = std::prev(surfaces.end());
        surfacesListChanged = true;
        surface->orderChanged();
    }

    // Rise its children
    for (LSurface *children : surface->children())
        raiseChildren(children);
}

wl_display *LCompositor::display()
{
    return LCompositor::compositor()->imp()->display;
}

wl_event_loop *LCompositor::eventLoop()
{
    return LCompositor::compositor()->imp()->eventLoop;
}

wl_event_source *LCompositor::addFdListener(int fd, void *userData, int (*callback)(int, unsigned int, void *), UInt32 flags)
{
    return wl_event_loop_add_fd(LCompositor::compositor()->imp()->eventLoop, fd, flags, callback, userData);
}

void LCompositor::removeFdListener(wl_event_source *source)
{
    wl_event_source_remove(source);
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
    for (std::list<LOutput*>::iterator it = imp()->outputs.begin(); it != imp()->outputs.end(); ++it)
        (*it)->repaint();
}

bool LCompositor::addOutput(LOutput *output)
{
    // Check if already initialized
    for (LOutput *o : outputs())
        if (o == output)
            return true;

    imp()->outputs.push_back(output);

    if (imp()->outputs.size() == 1)
        cursor()->imp()->setOutput(output);

    if (!output->imp()->initialize())
    {
        LLog::error("[Compositor] Failed to initialize output %s.", output->name());
        removeOutput(output);
        return false;
    }

    imp()->updateGreatestOutputScale();
    return true;
}

void LCompositor::removeOutput(LOutput *output)
{
    // Loop to check if output was added (initialized)
    for (std::list<LOutput*>::iterator it = imp()->outputs.begin(); it != imp()->outputs.end(); it++)
    {
        // Was initialized
        if (*it == output)
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
            imp()->graphicBackend->uninitializeOutput(output);

            while (output->imp()->state != LOutput::Uninitialized)
                usleep(1000);

            output->imp()->callLock.store(true);

            for (LSurface *s : surfaces())
                s->sendOutputLeaveEvent(output);

            for (LView *v : imp()->views)
                v->imp()->removeThread(v, (*it)->threadId());

            imp()->outputs.erase(it);

            // Remove all wl_outputs from clients
            for (LClient *c : clients())
            {
                for (GOutput *g : c->outputGlobals())
                {
                    if (output == g->output())
                    {
                        g->client()->imp()->outputGlobals.erase(g->imp()->clientLink);
                        g->imp()->lOutput = nullptr;

                        // Break because clients can bind to to a wl_output global just once
                        break;
                    }
                }
            }

            // Safely remove global
            imp()->removeGlobal(output->imp()->global);

            cursor()->imp()->intersectedOutputs.remove(output);

            if (cursor()->imp()->output == output)
                cursor()->imp()->output = nullptr;

            cursor()->move(1.f, 1.f);
            imp()->updateGreatestOutputScale();
            return;
        }
    }
}

const std::list<LSurface *> &LCompositor::surfaces() const
{
    return imp()->surfaces;
}

const std::list<LOutput *> &LCompositor::outputs() const
{
    return imp()->outputs;
}

const std::list<LClient *> &LCompositor::clients() const
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
    for (LClient *c : clients())
        if (c->client() == client)
            return c;
    return nullptr;
}

std::thread::id LCompositor::mainThreadId() const
{
    return imp()->threadId;
}
