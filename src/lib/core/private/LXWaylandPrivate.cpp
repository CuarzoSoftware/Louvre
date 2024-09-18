#include <private/LCompositorPrivate.h>
#include <private/LXWindowRolePrivate.h>
#include <private/LXWaylandPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LFactory.h>
#include <LClient.h>
#include <LUtils.h>
#include <LLog.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <wait.h>
#include <fcntl.h>
#include <cstring>
#include <cassert>

#include <xcb/xfixes.h>
#include <xcb/composite.h>
#include <xcb/res.h>

xcb_void_cookie_t LXWayland::LXWaylandPrivate::sendEventWithSize(UInt8 propagate, UInt32 winId, UInt32 eventMask, const void *event, UInt32 size)
{
    if (size == 32)
        return xcb_send_event(conn, propagate, winId, eventMask, (const char*)event);
    else if (size < 32)
    {
        char buf[32];
        memcpy(buf, event, size);
        memset(buf + size, 0, 32 - size);
        return xcb_send_event(conn, propagate, winId, eventMask, buf);
    }

    assert(false && "Event too long");
}

void LXWayland::LXWaylandPrivate::handleCreateNotify(void *event)
{
    const auto &ev { *static_cast<xcb_create_notify_event_t*>(event) };

    if (ev.window == window)
        return;

    static constexpr UInt32 values { XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(conn, ev.window, XCB_CW_EVENT_MASK, &values);

    const LXWindowRole::Params params
    {
        .xWaylandSurfaceRes = nullptr,
        .surface = nullptr,
        .winId = ev.window,
        .pos = LPoint(ev.x, ev.y),
        .size = LSize(ev.width, ev.height),
        .overrideRedirect = (bool)ev.override_redirect,
    };

    LFactory::createObject<LXWindowRole>(&params);
}

void LXWayland::LXWaylandPrivate::handleDestroyNotify(void *event)
{
    const auto &ev { *static_cast<xcb_destroy_notify_event_t*>(event) };
    const auto &win { windows.find(ev.window) };

    if (win == windows.end())
        return;

    // TODO: Unmap surface first?
    compositor()->onAnticipatedObjectDestruction(win->second);
    delete win->second;
}

void LXWayland::LXWaylandPrivate::handleConfigureRequest(void *event)
{
    const auto &ev { *static_cast<xcb_configure_request_event_t*>(event) };
    const auto &win { windows.find(ev.window) };

    if (win == windows.end())
        return;

    if ((ev.value_mask & (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT)) == 0)
        return;

    win->second->configureRequest({
        .pos = LPoint(
            ev.value_mask & XCB_CONFIG_WINDOW_X ? ev.x : win->second->imp()->pos.x(),
            ev.value_mask & XCB_CONFIG_WINDOW_Y ? ev.y : win->second->imp()->pos.y()),
        .size = LSize(
            ev.value_mask & XCB_CONFIG_WINDOW_WIDTH ? ev.width : win->second->imp()->size.w(),
            ev.value_mask & XCB_CONFIG_WINDOW_HEIGHT ? ev.height : win->second->imp()->size.h()
        )
    });
}

void LXWayland::LXWaylandPrivate::handleConfigureNotify(void *event)
{
    const auto &ev { *static_cast<xcb_configure_notify_event_t*>(event) };
    const auto &it { windows.find(ev.window) };

    if (it == windows.end())
        return;

    auto &win { *it->second->imp() };

    bool geometryChanged { false };

    if (win.pos.x() != ev.x)
    {
        win.pos.setX(ev.x);
        geometryChanged = true;
    }

    if (win.pos.y() != ev.y)
    {
        win.pos.setY(ev.y);
        geometryChanged = true;
    }

    if (win.size.w() != ev.width)
    {
        win.size.setW(ev.width);
        geometryChanged = true;
    }

    if (win.size.h() != ev.height)
    {
        win.size.setH(ev.height);
        geometryChanged = true;
    }

    win.setOverrideRedirect(ev.override_redirect);

    if (geometryChanged)
    {
        /* TODO: Notify geometry change */
    }
}

void LXWayland::LXWaylandPrivate::handleMapRequest(void *event)
{
    const auto &ev { *static_cast<xcb_map_request_event_t*>(event) };
    xcb_map_window(conn, ev.window);

    // TODO: Notify?
}

void LXWayland::LXWaylandPrivate::handleMapNotify(void *event)
{
    const auto &ev { *static_cast<xcb_map_notify_event_t*>(event) };
    const auto &it { windows.find(ev.window) };

    if (it == windows.end())
        return;

    auto &win { *it->second->imp() };
    win.setOverrideRedirect(ev.override_redirect);

    if (!win.overrideRedirect)
    {
        win.setWithdrawn(false);
        // TODO: restack;
    }

    win.mapped = true;

    if (win.role->surface())
        win.role->surface()->imp()->setMapped(win.role->surface()->hasBuffer());
}

void LXWayland::LXWaylandPrivate::handleClientMessage(void *event)
{
    auto &ev { *static_cast<xcb_client_message_event_t*>(event) };

    if (ev.type == atoms[WL_SURFACE_SERIAL])
        handleSurfaceSerialMessage(event);

    /* TODO
    else if (ev.type == atoms[WL_SURFACE_ID])
    else if (ev.type == atoms[WL_SURFACE_SERIAL])
    else if (ev.type == atoms[NET_WM_STATE])
    else if (ev.type == atoms[NET_WM_MOVERESIZE])
    else if (ev.type == atoms[WM_PROTOCOLS])
    else if (ev.type == atoms[NET_ACTIVE_WINDOW])
    else if (ev.type == atoms[NET_STARTUP_INFO] || ev.type == atoms[NET_STARTUP_INFO_BEGIN])
    else if (ev.type == atoms[WM_CHANGE_STATE])*/
}

void LXWayland::LXWaylandPrivate::handleSurfaceSerialMessage(void *event)
{
    auto &ev { *static_cast<xcb_client_message_event_t*>(event) };
    auto win = windows.find(ev.window);

    if (win == windows.end())
    {
        LLog::warning("[LXWaylandPrivate::handleSurfaceSerialMessage] Received client message WL_SURFACE_SERIAL but no X11 window %u.", ev.window);
        return;
    }

    const UInt32 lo { ev.data.data32[0] };
    const UInt32 hi { ev.data.data32[1] };
    win->second->imp()->serial = ((uint64_t)hi << 32) | lo;

    /* TODO: Handle role change */
}

int LXWayland::LXWaylandPrivate::x11EventHandler(int /*fd*/, UInt32 mask, void *data)
{
    auto &x { *static_cast<LXWayland::LXWaylandPrivate*>(data) };

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR))
    {
        //x.unit();
        return 0;
    }

    int flush { 0 };
    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event(x.conn)))
    {
        flush = 1;

        switch (event->response_type & 0x7f)
        {
        case XCB_CREATE_NOTIFY:
            x.handleCreateNotify(event);
            break;
        case XCB_DESTROY_NOTIFY:
            x.handleDestroyNotify(event);
            break;
        case XCB_CONFIGURE_REQUEST:
            x.handleConfigureRequest(event);
            break;
        case XCB_CONFIGURE_NOTIFY:
            x.handleConfigureNotify(event);
            break;
        case XCB_MAP_REQUEST:
            x.handleMapRequest(event);
            break;
        case XCB_MAP_NOTIFY:
            x.handleMapNotify(event);
            break;
        case XCB_UNMAP_NOTIFY:
            LLog::debug("XCB_UNMAP_NOTIFY");
            break;
        case XCB_PROPERTY_NOTIFY:
            LLog::debug("XCB_PROPERTY_NOTIFY");
            break;
        case XCB_CLIENT_MESSAGE:
            x.handleClientMessage(event);
            break;
        case XCB_FOCUS_IN:
            LLog::debug("XCB_FOCUS_IN");
            break;
        case 0:
            LLog::error("XCB ERROR");
            break;
        default:
            LLog::warning("UNHANDLED XCB EVENT");
            break;
        }

        free(event);
    }

    if (flush)
        xcb_flush(x.conn);

    return flush;
}

void LXWayland::LXWaylandPrivate::updateNetClientListStacking() noexcept
{
    const std::size_t n { windowsStack.size() };
    windowIds.clear();
    windowIds.reserve(n);

    for (LXWindowRole *win : windowsStack)
        windowIds.emplace_back(win->winId());

    xcb_change_property(
        conn,
        XCB_PROP_MODE_REPLACE,
        screen->root,
        atoms[NET_CLIENT_LIST_STACKING],
        XCB_ATOM_WINDOW,
        32, n,
        windowIds.data());
}

static const char xLockFmt[] { "/tmp/.X%d-lock" };
static const char xSocketFmt[] { "/tmp/.X11-unix/X%d" };
static const char xSocketDir[] { "/tmp/.X11-unix" };

static int openSocket(struct sockaddr_un *addr, size_t path_size)
{
    int fd, rc;
    socklen_t size = offsetof(struct sockaddr_un, sun_path) + path_size + 1;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0)
    {
        LLog::error("[LXWaylandPrivate::openSocket] Failed to create socket %c%s",
                      addr->sun_path[0] ? addr->sun_path[0] : '@',
                      addr->sun_path + 1);
        return -1;
    }

    if (!setCloexec(fd, true))
    {
        close(fd);
        return -1;
    }

    if (addr->sun_path[0])
        unlink(addr->sun_path);

    if (bind(fd, (struct sockaddr*)addr, size) < 0)
    {
        rc = errno;
        LLog::error("[LXWaylandPrivate::openSocket] Failed to bind socket %c%s",
                      addr->sun_path[0] ? addr->sun_path[0] : '@',
                      addr->sun_path + 1);
        goto cleanup;
    }

    if (listen(fd, 1) < 0)
    {
        rc = errno;
        LLog::error("[LXWaylandPrivate::openSocket] Failed to listen to socket %c%s",
                      addr->sun_path[0] ? addr->sun_path[0] : '@',
                      addr->sun_path + 1);
        goto cleanup;
    }

    return fd;

cleanup:
    close(fd);
    if (addr->sun_path[0])
        unlink(addr->sun_path);
    errno = rc;
    return -1;
}

static bool checkSocketDir()
{
    struct stat buf;

    if (lstat(xSocketDir, &buf))
    {
        LLog::error("[LXWaylandPrivate::checkSocketDir] Failed to stat %s", xSocketDir);
        return false;
    }

    if (!(buf.st_mode & S_IFDIR))
    {
        LLog::error("[LXWaylandPrivate::checkSocketDir] %s is not a directory", xSocketDir);
        return false;
    }

    if (!((buf.st_uid == 0) || (buf.st_uid == getuid())))
    {
        LLog::error("[LXWaylandPrivate::checkSocketDir] %s not owned by root or us", xSocketDir);
        return false;
    }

    if (!(buf.st_mode & S_ISVTX) && (buf.st_mode & (S_IWGRP | S_IWOTH)))
    {
        LLog::error("[LXWaylandPrivate::checkSocketDir] Sticky bit not set on %s", xSocketDir);
        return false;
    }

    return true;
}

static bool openSockets(int socks[2], int display)
{
    sockaddr_un addr
    {
        .sun_family = AF_UNIX
    };

    size_t pathSize;

    if (mkdir(xSocketDir, 0755) == 0)
        LLog::debug("[LXWaylandPrivate::openSockets] Created %s ourselves -- other users will be unable to create X11 UNIX sockets of their own", xSocketDir);
    else if (errno != EEXIST)
    {
        LLog::error("[LXWaylandPrivate::openSockets] Unable to mkdir %s", xSocketDir);
        return false;
    }
    else if (!checkSocketDir())
        return false;

#ifdef __linux__
    addr.sun_path[0] = 0;
    pathSize = snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 1, xSocketFmt, display);
#else
    pathSize = snprintf(addr.sun_path, sizeof(addr.sun_path), socket_fmt2, display);
#endif
    socks[0] = openSocket(&addr, pathSize);

    if (socks[0] < 0)
        return false;

    pathSize = snprintf(addr.sun_path, sizeof(addr.sun_path), xSocketFmt, display);
    socks[1] = openSocket(&addr, pathSize);

    if (socks[1] < 0)
    {
        close(socks[0]);
        socks[0] = -1;
        return false;
    }

    return true;
}

static int openDisplaySockets(int socks[2])
{
    int lockFd, display;
    char lockName[64];

    for (display = 0; display <= 32; display++)
    {
        snprintf(lockName, sizeof(lockName), xLockFmt, display);

        if ((lockFd = open(lockName, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0444)) >= 0)
        {
            if (!openSockets(socks, display))
            {
                unlink(lockName);
                close(lockFd);
                continue;
            }

            char pid[12];
            snprintf(pid, sizeof(pid), "%10d", getpid());

            if (write(lockFd, pid, sizeof(pid) - 1) != sizeof(pid) - 1)
            {
                unlink(lockName);
                close(lockFd);
                continue;
            }

            close(lockFd);
            break;
        }

        if ((lockFd = open(lockName, O_RDONLY | O_CLOEXEC)) < 0)
            continue;

        char pid[12] = { 0 }, *end_pid;
        ssize_t bytes = read(lockFd, pid, sizeof(pid) - 1);
        close(lockFd);

        if (bytes != sizeof(pid) - 1)
            continue;

        long int read_pid;
        read_pid = strtol(pid, &end_pid, 10);

        if (read_pid < 0 || read_pid > INT32_MAX || end_pid != pid + sizeof(pid) - 2)
            continue;

        errno = 0;

        if (kill((pid_t)read_pid, 0) != 0 && errno == ESRCH)
        {
            if (unlink(lockName) != 0)
                continue;

            // retry
            display--;
            continue;
        }
    }

    if (display > 32)
    {
        LLog::error("[LXWaylandPrivate::openDisplaySockets] No X display available in the first 33.");
        return -1;
    }

    return display;
}

static void unlinkDisplaySockets(int display)
{
    char sun_path[64];

    snprintf(sun_path, sizeof(sun_path), xSocketFmt, display);
    unlink(sun_path);

#ifndef __linux__
    snprintf(sun_path, sizeof(sun_path), socket_fmt2, display);
    unlink(sun_path);
#endif

    snprintf(sun_path, sizeof(sun_path), xLockFmt, display);
    unlink(sun_path);
}

void LXWayland::LXWaylandPrivate::init()
{
    if (state != Uninitialized)
        return;

    state = Initializing;

    if (!initXWaylandPath())
        goto fail;

    // This also calls initServer()
    if (!initDisplay())
        goto fail;

    return;
fail:
    unit();
}

void LXWayland::LXWaylandPrivate::unit()
{
    unitServer();
    unitDisplay();
    unitXWaylandPath();

    const State endState { state };

    state = Uninitialized;

    if (endState == Initializing)
        xWayland()->onFail();
    else if (endState == Initialized)
        xWayland()->onStop();
}

bool LXWayland::LXWaylandPrivate::initXWaylandPath() noexcept
{
    xWaylandFullPath = getenvString("LOUVRE_XWAYLAND_PATH");

    if (!xWaylandFullPath.empty())
    {
        if (!isExecutable(xWaylandFullPath))
        {
            LLog::warning("[LXWaylandPrivate::initXWaylandPath] User defined LOUVRE_XWAYLAND_PATH=%s is invalid. Using PATH instead.", xWaylandFullPath.c_str());
            xWaylandFullPath.clear();
        }
    }

    if (xWaylandFullPath.empty())
        xWaylandFullPath = whereIsExecutable("Xwayland");

    if (xWaylandFullPath.empty())
        LLog::error("[LXWaylandPrivate::initXWaylandPath] The Xwayland executable could not be found. Please ensure that Xwayland is included in your PATH environment variable or use LOUVRE_XWAYLAND_PATH to provide the full path to the executable.");

    return !xWaylandFullPath.empty();
}

bool LXWayland::LXWaylandPrivate::initDisplay() noexcept
{
    display = openDisplaySockets(x_fd);

    if (display < 0)
    {
        LLog::debug("[LXWaylandPrivate::initDisplay] Failed to open X Display sockets.");
        return false;
    }

    displayName = ":" + std::to_string(display);
    LLog::debug("[LXWaylandPrivate::initDisplay] X DISPLAY=%s.", displayName.c_str());

    x11IdleStartEventSource = wl_event_loop_add_idle(
        compositor()->imp()->auxEventLoop,
        [](void *data)
        {
            auto &x { *static_cast<LXWayland::LXWaylandPrivate*>(data) };
            x.x11IdleStartEventSource = nullptr;
            if (!x.initServer())
                x.unit();
        }, this);

    return x11IdleStartEventSource != nullptr;
}

bool LXWayland::LXWaylandPrivate::initServer()
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, wl_fd) != 0)
    {
        LLog::error("[LXWaylandPrivate::initServer] socketpair failed. %s", strerror(errno));
        return false;
    }

    if (!setCloexec(wl_fd[0], true) || !setCloexec(wl_fd[1], true))
    {
        LLog::error("[LXWaylandPrivate::initServer] Failed to set O_CLOEXEC on WL socket.");
        return false;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, wm_fd) != 0)
    {
        LLog::error("[LXWaylandPrivate::initServer] socketpair failed. %s", strerror(errno));
        return false;
    }

    if (!setCloexec(wm_fd[0], true) || !setCloexec(wm_fd[1], true))
    {
        LLog::error("[LXWaylandPrivate::initServer] Failed to set O_CLOEXEC on WM socket.");
        return false;
    }

    wl_client *wlClient { wl_client_create(LCompositor::display(), wl_fd[0]) };

    if (!wlClient)
    {
        LLog::error("[LXWaylandPrivate::initServer] wl_client_create failed. %s", strerror(errno));
        return false;
    }

    client.reset(compositor()->getClientFromNativeResource(wlClient));

    if (!client)
    {
        LLog::fatal("[LXWaylandPrivate::initServer] Failed to get XWayland client.");
        return false;
    }

    wl_fd[0] = -1;

    // TODO
    //server->client_destroy.notify = handle_client_destroy;
    //wl_client_add_destroy_listener(server->client, &server->client_destroy);

    int notifyFd[2];

    if (pipe(notifyFd) == -1)
    {
        LLog::error("[LXWaylandPrivate::initServer] pipe failed. %s", strerror(errno));
        return false;
    }

    if (!setCloexec(notifyFd[0], true))
    {
        LLog::error("[LXWaylandPrivate::initServer] Failed to set CLOEXEC on FD.");
        close(notifyFd[0]);
        close(notifyFd[1]);
        return false;
    }

    xInitEventSource = Louvre::compositor()->addFdListener(notifyFd[0], this, &onServerReady);

    pid = fork();

    if (pid < 0)
    {
        LLog::error("[LXWaylandPrivate::initServer] fork failed. %s", strerror(errno));
        close(notifyFd[0]);
        close(notifyFd[1]);
        return false;
    }
    else if (pid == 0)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            LLog::error("[LXWaylandPrivate::initServer] second fork failed. %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
            execXWayland(notifyFd[1]);

        exit(EXIT_SUCCESS);
    }

    close(notifyFd[1]);
    close(wl_fd[1]);

    if (wm_fd[1] >= 0)
        close(wm_fd[1]);

    wl_fd[1] = wm_fd[1] = -1;

    return true;
}

void LXWayland::LXWaylandPrivate::execXWayland(int notifyFd)
{
    if (!setCloexec(x_fd[0], false) || !setCloexec(x_fd[1], false) || !setCloexec(wl_fd[1], false))
    {
        LLog::error("[LXWaylandPrivate::execXWayland] Failed to unset CLOEXEC on x FD.");
        exit(EXIT_FAILURE);
    }

    if (!setCloexec(wm_fd[1], false))
    {
        LLog::error("[LXWaylandPrivate::execXWayland] Failed to unset CLOEXEC on wl FD");
        exit(EXIT_FAILURE);
    }

    const std::string listenfd0 { std::to_string(x_fd[0]) };
    const std::string listenfd1 { std::to_string(x_fd[1]) };
    const std::string displayfd { std::to_string(notifyFd) };
    const std::string wmfd      { std::to_string(wm_fd[1]) };

    const char * const args[]
    {
        "Xwayland",
        displayName.c_str(),
        "-rootless",
        "-core",
        "-terminate",
        "0",
        "-listenfd",
        listenfd0.c_str(),
        "-listenfd",
        listenfd1.c_str(),
        "-displayfd",
        displayfd.c_str(),
        "-wm",
        wmfd.c_str(),
        nullptr
    };

    const std::string waylandSocketFdStr { std::to_string(wl_fd[1]) };
    setenv("WAYLAND_SOCKET", waylandSocketFdStr.c_str(), 1);

    LLog::debug("[LXWaylandPrivate::execXWayland] Starting Xwayland on :%d", display);

    int nullDev { open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0666) };
    if (nullDev < 0)
    {
        LLog::error("[LXWaylandPrivate::execXWayland] XWayland: failed to open /dev/null");
        exit(EXIT_FAILURE);
    }

    dup2(nullDev, STDOUT_FILENO);
    dup2(nullDev, STDERR_FILENO);

    execvp(xWaylandFullPath.c_str(), (char*const*)args);

    LLog::error("[LXWaylandPrivate::execXWayland] Failed to exec %s", xWaylandFullPath.c_str());
    close(nullDev);
    exit(EXIT_FAILURE);
}

int LXWayland::LXWaylandPrivate::onServerReady(int fd, UInt32 mask, void *data)
{
    auto &x { *static_cast<LXWayland::LXWaylandPrivate*>(data) };

    if (mask & WL_EVENT_READABLE)
    {
        char buf[64];
        const ssize_t n { read(fd, buf, sizeof(buf)) };

        if (n < 0 && errno != EINTR)
        {
            LLog::error("[LXWaylandPrivate::onServerReady] Read from Xwayland display_fd failed: %s.", strerror(errno));
            mask = 0;
        } else if (n <= 0 || buf[n-1] != '\n')
            return 1;
    }

    while (waitpid(x.pid, NULL, 0) < 0)
    {
        if (errno == EINTR)
            continue;

        LLog::error("[LXWaylandPrivate::onServerReady] waitpid for Xwayland fork failed: %s.", strerror(errno));
        goto error;
    }

    if (!(mask & WL_EVENT_READABLE))
    {
        assert(mask & WL_EVENT_HANGUP);
        LLog::error("[LXWaylandPrivate::onServerReady] Xwayland startup failed, not setting up xwm: %s.", strerror(errno));
        goto error;
    }

    LLog::debug("[LXWaylandPrivate::onServerReady] X server is ready.");

    close(fd);
    wl_event_source_remove(x.xInitEventSource);
    x.xInitEventSource = NULL;
    x.initWM();
    return 0;
error:
    close(fd);
    x.unit();
    return 0;
}

void LXWayland::LXWaylandPrivate::unitXWaylandPath() noexcept
{
    xWaylandFullPath.clear();
}

void LXWayland::LXWaylandPrivate::unitDisplay() noexcept
{
    if (x11IdleStartEventSource)
    {
        wl_event_source_remove(x11IdleStartEventSource);
        x11IdleStartEventSource = nullptr;
    }

    if (display == -1)
        return;

    for (int i = 0; i < 2; i++)
    {
        if (x_fd[i] >= 0)
        {
            close(x_fd[i]);
            x_fd[i] = -1;
        }
    }

    unlinkDisplaySockets(display);
    display = -1;
    displayName.clear();
}

void LXWayland::LXWaylandPrivate::unitServer()
{
    if (display == -1)
        return;

    if (client)
        wl_client_destroy(client->client());

    if (xInitEventSource)
    {
        wl_event_source_remove(xInitEventSource);
        xInitEventSource = nullptr;
    }

    for (int i = 0; i < 2; i++)
    {
        if (wl_fd[i] >= 0)
        {
            close(wl_fd[i]);
            wl_fd[i] = -1;
        }

        if (wm_fd[i] >= 0)
        {
            close(wm_fd[i]);
            wm_fd[i] = -1;
        }
    }
}

void LXWayland::LXWaylandPrivate::initWM()
{
    conn = xcb_connect_to_fd(wm_fd[0], NULL);

    const int rc { xcb_connection_has_error(conn) };

    if (rc)
    {
        LLog::error("xcb connect failed: %d", rc);
        unit();
        return;
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    x11EventSource = Louvre::compositor()->addFdListener(wm_fd[0], this, &x11EventHandler);
    wl_event_source_check(x11EventSource);

    wmGetResources();
    wmGetVisualAndColormap();
    wmGetRenderFormat();

    UInt32 values[]
    {
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_PROPERTY_CHANGE,
    };

    xcb_change_window_attributes(conn, screen->root, XCB_CW_EVENT_MASK, values);
    xcb_composite_redirect_subwindows(conn, screen->root, XCB_COMPOSITE_REDIRECT_MANUAL);

    xcb_atom_t supported[]
    {
        atoms[NET_WM_STATE],
        atoms[NET_ACTIVE_WINDOW],
        atoms[NET_WM_MOVERESIZE],
        atoms[NET_WM_STATE_FOCUSED],
        atoms[NET_WM_STATE_MODAL],
        atoms[NET_WM_STATE_FULLSCREEN],
        atoms[NET_WM_STATE_MAXIMIZED_VERT],
        atoms[NET_WM_STATE_MAXIMIZED_HORZ],
        atoms[NET_WM_STATE_HIDDEN],
        atoms[NET_CLIENT_LIST],
        atoms[NET_CLIENT_LIST_STACKING],
    };

    xcb_change_property(conn,
                        XCB_PROP_MODE_REPLACE,
                        screen->root,
                        atoms[NET_SUPPORTED],
                        XCB_ATOM_ATOM,
                        32,
                        sizeof(supported)/sizeof(*supported),
                        supported);

    xcb_flush(conn);

    wmSetNetActiveWindow(XCB_WINDOW_NONE);

    /* TODO: Add clipboard, DND and move Xwayland shell global creation here */

    wmCreateWindow();
    xcb_flush(conn);

    state = Initialized;
    xWayland()->onStart();
}

void LXWayland::LXWaylandPrivate::wmGetResources() noexcept
{
    xcb_prefetch_extension_data(conn, &xcb_xfixes_id);
    xcb_prefetch_extension_data(conn, &xcb_composite_id);
    xcb_prefetch_extension_data(conn, &xcb_res_id);

    xcb_intern_atom_cookie_t cookies[ATOM_LAST];

    for (size_t i = 0; i < ATOM_LAST; i++)
        cookies[i] = xcb_intern_atom(conn, 0, strlen(atomsMap[i]), atomsMap[i]);

    for (size_t i = 0; i < ATOM_LAST; i++)
    {
        xcb_generic_error_t *error;
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookies[i], &error);

        if (reply && !error)
            atoms[i] = reply->atom;

        free(reply);

        if (error)
        {
            LLog::error("[LXWaylandPrivate::wmGetResources] Could not resolve atom %s, x11 error code %d", atomsMap[i], error->error_code);
            free(error);
            return;
        }
    }

    xfixes = xcb_get_extension_data(conn, &xcb_xfixes_id);

    if (!xfixes || !xfixes->present)
        LLog::debug("[LXWaylandPrivate::wmGetResources] xfixes not available.");

    xcb_xfixes_query_version_cookie_t xfixesCookie = xcb_xfixes_query_version(conn, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
    xcb_xfixes_query_version_reply_t *xfixesReply = xcb_xfixes_query_version_reply(conn, xfixesCookie, NULL);
    LLog::debug("[LXWaylandPrivate::wmGetResources] xfixes version: %" PRIu32 ".%" PRIu32, xfixesReply->major_version, xfixesReply->minor_version);
    xfixesMajor = xfixesReply->major_version;
    free(xfixesReply);

    const xcb_query_extension_reply_t *xres = xcb_get_extension_data(conn, &xcb_res_id);

    if (!xres || !xres->present)
        return;

    xcb_res_query_version_cookie_t xresCookie = xcb_res_query_version(conn, XCB_RES_MAJOR_VERSION, XCB_RES_MINOR_VERSION);
    xcb_res_query_version_reply_t *xresReply = xcb_res_query_version_reply(conn, xresCookie, NULL);

    if (xresReply == NULL)
        return;

    LLog::debug("[LXWaylandPrivate::wmGetResources] xres version: %" PRIu32 ".%" PRIu32, xresReply->server_major, xresReply->server_minor);

    if (xresReply->server_major > 1 || (xresReply->server_major == 1 && xresReply->server_minor >= 2))
        this->xres = xres;

    free(xresReply);
}

void LXWayland::LXWaylandPrivate::wmGetVisualAndColormap() noexcept
{
    xcb_depth_iterator_t depthIt;
    xcb_visualtype_iterator_t visualtypeIt;
    xcb_visualtype_t *visualtype;

    depthIt = xcb_screen_allowed_depths_iterator(screen);
    visualtype = NULL;

    while (depthIt.rem > 0)
    {
        if (depthIt.data->depth == 32)
        {
            visualtypeIt = xcb_depth_visuals_iterator(depthIt.data);
            visualtype = visualtypeIt.data;
            break;
        }

        xcb_depth_next(&depthIt);
    }

    if (visualtype == NULL)
    {
        LLog::warning("[LXWaylandPrivate::wmGetVisualAndColormap] No 32 bit visualtype.");
        return;
    }

    visualId = visualtype->visual_id;
    colormap = xcb_generate_id(conn);
    xcb_create_colormap(conn, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visualId);
}

void LXWayland::LXWaylandPrivate::wmGetRenderFormat() noexcept
{
    xcb_render_query_pict_formats_cookie_t cookie = xcb_render_query_pict_formats(conn);
    xcb_render_query_pict_formats_reply_t *reply = xcb_render_query_pict_formats_reply(conn, cookie, NULL);

    if (!reply)
    {
        LLog::error("[LXWaylandPrivate::wmGetRenderFormat] Did not get any reply from xcb_render_query_pict_formats.");
        return;
    }

    xcb_render_pictforminfo_iterator_t it = xcb_render_query_pict_formats_formats_iterator(reply);
    xcb_render_pictforminfo_t *format = NULL;

    while (it.rem > 0)
    {
        if (it.data->depth == 32)
        {
            format = it.data;
            break;
        }

        xcb_render_pictforminfo_next(&it);
    }

    if (format == NULL)
    {
        LLog::fatal("[LXWaylandPrivate::wmGetRenderFormat] No 32 bit render format.");
        free(reply);
        return;
    }

    renderFormatId = format->id;
    free(reply);
}

void LXWayland::LXWaylandPrivate::wmSetNetActiveWindow(UInt32 winId) noexcept
{
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root, atoms[NET_ACTIVE_WINDOW], atoms[WINDOW], 32, 1, &winId);
}

void LXWayland::LXWaylandPrivate::wmCreateWindow() noexcept
{
    static const char name[] = "Louvre";

    window = xcb_generate_id(conn);

    xcb_create_window(
        conn,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0,
        10, 10,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        0, NULL);

    xcb_change_property(
        conn,
        XCB_PROP_MODE_REPLACE,
        window,
        atoms[NET_WM_NAME],
        atoms[UTF8_STRING],
        8,
        strlen(name), name);

    xcb_change_property(
        conn,
        XCB_PROP_MODE_REPLACE,
        screen->root,
        atoms[NET_SUPPORTING_WM_CHECK],
        XCB_ATOM_WINDOW,
        32,
        1, &window);

    xcb_change_property(
        conn,
        XCB_PROP_MODE_REPLACE,
        window,
        atoms[NET_SUPPORTING_WM_CHECK],
        XCB_ATOM_WINDOW,
        32,
        1, &window);

    xcb_set_selection_owner(conn, window, atoms[WM_S0], XCB_CURRENT_TIME);
    xcb_set_selection_owner(conn, window, atoms[NET_WM_CM_S0], XCB_CURRENT_TIME);
}
