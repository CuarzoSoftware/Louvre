#include <cassert>
#include <private/LXWaylandPrivate.h>
#include <LCompositor.h>
#include <sys/socket.h>
#include <cstring>
#include <LLog.h>
#include <LUtils.h>
#include <sys/stat.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/un.h>

static const char lock_fmt[] = "/tmp/.X%d-lock";
static const char socket_fmt[] = "/tmp/.X11-unix/X%d";
static const char socket_dir[] = "/tmp/.X11-unix";

static int open_socket(struct sockaddr_un *addr, size_t path_size) {
    int fd, rc;
    socklen_t size = offsetof(struct sockaddr_un, sun_path) + path_size + 1;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        //wlr_log_errno(WLR_ERROR, "Failed to create socket %c%s",
        //              addr->sun_path[0] ? addr->sun_path[0] : '@',
        //              addr->sun_path + 1);
        return -1;
    }
    if (!setCloexec(fd, true)) {
        close(fd);
        return -1;
    }

    if (addr->sun_path[0]) {
        unlink(addr->sun_path);
    }
    if (bind(fd, (struct sockaddr*)addr, size) < 0) {
        rc = errno;
        //wlr_log_errno(WLR_ERROR, "Failed to bind socket %c%s",
        //              addr->sun_path[0] ? addr->sun_path[0] : '@',
        //              addr->sun_path + 1);
        goto cleanup;
    }
    if (listen(fd, 1) < 0) {
        rc = errno;
        //wlr_log_errno(WLR_ERROR, "Failed to listen to socket %c%s",
        //              addr->sun_path[0] ? addr->sun_path[0] : '@',
        //              addr->sun_path + 1);
        goto cleanup;
    }

    return fd;

cleanup:
    close(fd);
    if (addr->sun_path[0]) {
        unlink(addr->sun_path);
    }
    errno = rc;
    return -1;
}

static bool check_socket_dir(void) {
    struct stat buf;

    if (lstat(socket_dir, &buf)) {
        //wlr_log_errno(WLR_ERROR, "Failed to stat %s", socket_dir);
        return false;
    }
    if (!(buf.st_mode & S_IFDIR)) {
        //wlr_log(WLR_ERROR, "%s is not a directory", socket_dir);
        return false;
    }
    if (!((buf.st_uid == 0) || (buf.st_uid == getuid()))) {
        //wlr_log(WLR_ERROR, "%s not owned by root or us", socket_dir);
        return false;
    }
    if (!(buf.st_mode & S_ISVTX)) {
        /* we can deal with no sticky bit... */
        if ((buf.st_mode & (S_IWGRP | S_IWOTH))) {
            /* but not if other users can mess with our sockets */
            //wlr_log(WLR_ERROR, "sticky bit not set on %s", socket_dir);
            return false;
        }
    }
    return true;
}

static bool open_sockets(int socks[2], int display) {
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    size_t path_size;

    if (mkdir(socket_dir, 0755) == 0) {
        /*wlr_log(WLR_INFO, "Created %s ourselves -- other users will "
                          "be unable to create X11 UNIX sockets of their own",
                socket_dir);*/
    } else if (errno != EEXIST) {
        //wlr_log_errno(WLR_ERROR, "Unable to mkdir %s", socket_dir);
        return false;
    } else if (!check_socket_dir()) {
        return false;
    }

#ifdef __linux__
    addr.sun_path[0] = 0;
    path_size = snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 1, socket_fmt, display);
#else
    path_size = snprintf(addr.sun_path, sizeof(addr.sun_path), socket_fmt2, display);
#endif
    socks[0] = open_socket(&addr, path_size);
    if (socks[0] < 0) {
        return false;
    }

    path_size = snprintf(addr.sun_path, sizeof(addr.sun_path), socket_fmt, display);
    socks[1] = open_socket(&addr, path_size);
    if (socks[1] < 0) {
        close(socks[0]);
        socks[0] = -1;
        return false;
    }

    return true;
}

int open_display_sockets(int socks[2]) {
    int lock_fd, display;
    char lock_name[64];

    for (display = 0; display <= 32; display++) {
        snprintf(lock_name, sizeof(lock_name), lock_fmt, display);
        if ((lock_fd = open(lock_name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0444)) >= 0) {
            if (!open_sockets(socks, display)) {
                unlink(lock_name);
                close(lock_fd);
                continue;
            }
            char pid[12];
            snprintf(pid, sizeof(pid), "%10d", getpid());
            if (write(lock_fd, pid, sizeof(pid) - 1) != sizeof(pid) - 1) {
                unlink(lock_name);
                close(lock_fd);
                continue;
            }
            close(lock_fd);
            break;
        }

        if ((lock_fd = open(lock_name, O_RDONLY | O_CLOEXEC)) < 0) {
            continue;
        }

        char pid[12] = { 0 }, *end_pid;
        ssize_t bytes = read(lock_fd, pid, sizeof(pid) - 1);
        close(lock_fd);

        if (bytes != sizeof(pid) - 1) {
            continue;
        }
        long int read_pid;
        read_pid = strtol(pid, &end_pid, 10);
        if (read_pid < 0 || read_pid > INT32_MAX || end_pid != pid + sizeof(pid) - 2) {
            continue;
        }
        errno = 0;
        if (kill((pid_t)read_pid, 0) != 0 && errno == ESRCH) {
            if (unlink(lock_name) != 0) {
                continue;
            }
            // retry
            display--;
            continue;
        }
    }

    if (display > 32) {
        //wlr_log(WLR_ERROR, "No display available in the first 33");
        return -1;
    }

    return display;
}


static void setupWM(LXWayland::LXWaylandPrivate *x)
{
    x->conn = xcb_connect_to_fd(x->wm_fd[0], NULL);

    int rc = xcb_connection_has_error(x->conn);

    if (rc) {
        LLog::error("xcb connect failed: %d", rc);
        //free(xwm);
        return;
    }

    xcb_screen_iterator_t screen_iterator =
        xcb_setup_roots_iterator(xcb_get_setup(x->conn));
    x->screen = screen_iterator.data;

    Louvre::compositor()->addFdListener(x->wm_fd[0], x, [](int, unsigned int, void *data) -> int
    {
        static int i = 0;
        LLog::debug("EVENT %d", i);
        i++;
        return 0;
    });


}
static int xserver_handle_ready(int fd, uint32_t mask, void *data)
{
    LXWayland::LXWaylandPrivate *x = (LXWayland::LXWaylandPrivate*)data;

    if (mask & WL_EVENT_READABLE)
    {
        /* Xwayland writes to the pipe twice, so if we close it too early
         * it's possible the second write will fail and Xwayland shuts down.
         * Make sure we read until end of line marker to avoid this.
         */
        char buf[64];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0 && errno != EINTR)
        {
            /* Clear mask to signal start failure after reaping child */
            //wlr_log_errno(WLR_ERROR, "read from Xwayland display_fd failed");
            mask = 0;
        } else if (n <= 0 || buf[n-1] != '\n')
        {
            /* Returning 1 here means recheck and call us again if required. */
            return 1;
        }
    }

    while (waitpid(x->pid, NULL, 0) < 0)
    {
        if (errno == EINTR)
        {
            continue;
        }
        //wlr_log_errno(WLR_ERROR, "waitpid for Xwayland fork failed");
        goto error;
    }
    /* Xwayland will only write on the fd once it has finished its
     * initial setup. Getting an event here without READABLE means
     * the server end failed.
     */
    if (!(mask & WL_EVENT_READABLE))
    {
        assert(mask & WL_EVENT_HANGUP);
        //wlr_log(WLR_ERROR, "Xwayland startup failed, not setting up xwm");
        goto error;
    }

    //wlr_log(WLR_DEBUG, "Xserver is ready");

    close(fd);
    wl_event_source_remove(x->xInitEventSource);
    x->xInitEventSource = NULL;

    /*
    server->ready = true;

    struct wlr_xwayland_server_ready_event event = {
        .server = server,
        .wm_fd = server->wm_fd[0],
    };
    wl_signal_emit_mutable(&server->events.ready, &event);
*/

    setupWM(x);

    /* We removed the source, so don't need recheck */
    return 0;

error:
    /* clean up */
    close(fd);
    //server_finish_process(server);
    //server_finish_display(server);
    return 0;
}

static void exec_xwayland(LXWayland::LXWaylandPrivate *x, int notify_fd) {

    if (!setCloexec(x->x_fd[0], false) ||
        !setCloexec(x->x_fd[1], false) ||
        !setCloexec(x->wl_fd[1], false))
    {
        //wlr_log(WLR_ERROR, "Failed to unset CLOEXEC on FD");
        exit(EXIT_FAILURE);
    }
    if (x->enableWM && !setCloexec(x->wm_fd[1], false)) {
        //wlr_log(WLR_ERROR, "Failed to unset CLOEXEC on FD");
        exit(EXIT_FAILURE);
    }

    char *argv[64] = {0};
    size_t i = 0;

    char listenfd0[16], listenfd1[16], displayfd[16];
    snprintf(listenfd0, sizeof(listenfd0), "%d", x->x_fd[0]);
    snprintf(listenfd1, sizeof(listenfd1), "%d", x->x_fd[1]);
    snprintf(displayfd, sizeof(displayfd), "%d", notify_fd);

    argv[i++] = "Xwayland";
    argv[i++] = (char*)x->xDisplayName.c_str();
    argv[i++] = "-rootless";
    argv[i++] = "-core";

    argv[i++] = "-terminate";
#if HAVE_XWAYLAND_TERMINATE_DELAY
    char terminate_delay[16];
    if (server->options.terminate_delay > 0) {
        snprintf(terminate_delay, sizeof(terminate_delay), "%d",
                 server->options.terminate_delay);
        argv[i++] = terminate_delay;
    }
#endif

#if HAVE_XWAYLAND_LISTENFD
    argv[i++] = "-listenfd";
    argv[i++] = listenfd0;
    argv[i++] = "-listenfd";
    argv[i++] = listenfd1;
#else
    argv[i++] = "-listen";
    argv[i++] = listenfd0;
    argv[i++] = "-listen";
    argv[i++] = listenfd1;
#endif
    argv[i++] = "-displayfd";
    argv[i++] = displayfd;

    char wmfd[16];
    if (x->enableWM) {
        snprintf(wmfd, sizeof(wmfd), "%d", x->wm_fd[1]);
        argv[i++] = "-wm";
        argv[i++] = wmfd;
    }

#if HAVE_XWAYLAND_NO_TOUCH_POINTER_EMULATION
    if (server->options.no_touch_pointer_emulation) {
        argv[i++] = "-noTouchPointerEmulation";
    }
#else
    //server->options.no_touch_pointer_emulation = false;
#endif

#if HAVE_XWAYLAND_FORCE_XRANDR_EMULATION
    if (server->options.force_xrandr_emulation) {
        argv[i++] = "-force-xrandr-emulation";
    }
#else
    //server->options.force_xrandr_emulation = false;
#endif

    argv[i++] = NULL;

    assert(i < sizeof(argv) / sizeof(argv[0]));

    char wayland_socket_str[16];
    snprintf(wayland_socket_str, sizeof(wayland_socket_str), "%d", x->wl_fd[1]);
    setenv("WAYLAND_SOCKET", wayland_socket_str, true);

    //wlr_log(WLR_INFO, "Starting Xwayland on :%d", server->display);

    // Closes stdout/stderr depending on log verbosity
    //enum wlr_log_importance verbosity = wlr_log_get_verbosity();
    int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0666);
    if (devnull < 0) {
        //wlr_log_errno(WLR_ERROR, "XWayland: failed to open /dev/null");
        _exit(EXIT_FAILURE);
    }
    /*
    if (verbosity < WLR_INFO) {
        dup2(devnull, STDOUT_FILENO);
    }
    if (verbosity < WLR_ERROR) {
        dup2(devnull, STDERR_FILENO);
    }*/

    const char *xwayland_path = getenv("WLR_XWAYLAND");
    if (xwayland_path) {
        //wlr_log(WLR_INFO, "Using Xwayland binary '%s' due to WLR_XWAYLAND",
        //        xwayland_path);
    } else {
        xwayland_path = "/usr/bin/Xwayland";
    }

    // This returns if and only if the call fails
    execvp(xwayland_path, argv);

    //wlr_log_errno(WLR_ERROR, "failed to exec %s", xwayland_path);
    close(devnull);
    _exit(EXIT_FAILURE);
}

bool LXWayland::LXWaylandPrivate::start()
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, wl_fd) != 0)
    {
        LLog::error("[LXWaylandPrivate::start] socketpair failed. %s", strerror(errno));
        //server_finish_process(server);
        return false;
    }

    if (!setCloexec(wl_fd[0], true) || !setCloexec(wl_fd[1], true))
    {
        LLog::error("[LXWaylandPrivate::start] Failed to set O_CLOEXEC on socket.");
        //server_finish_process(server);
        return false;
    }

    if (enableWM)
    {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, wm_fd) != 0)
        {
            LLog::error("[LXWaylandPrivate::start] socketpair failed. %s", strerror(errno));
            //server_finish_process(server);
            return false;
        }

        if (!setCloexec(wm_fd[0], true) || !setCloexec(wm_fd[1], true))
        {
            LLog::error("[LXWaylandPrivate::start] Failed to set O_CLOEXEC on socket.");
            //server_finish_process(server);
            return false;
        }
    }

    //server->server_start = time(NULL);

    wlClient = wl_client_create(LCompositor::display(), wl_fd[0]);

    if (!wlClient)
    {
        LLog::error("[LXWaylandPrivate::start] wl_client_create failed. %s", strerror(errno));
        //server_finish_process(server);
        return false;
    }

    wl_fd[0] = -1; /* not ours anymore */

    //server->client_destroy.notify = handle_client_destroy;
    //wl_client_add_destroy_listener(server->client, &server->client_destroy);

    Int32 notify_fd[2];

    if (pipe(notify_fd) == -1)
    {
        LLog::error("[LXWaylandPrivate::start] pipe failed. %s", strerror(errno));
        //server_finish_process(server);
        return false;
    }

    if (!setCloexec(notify_fd[0], true))
    {
        LLog::error("[LXWaylandPrivate::start] Failed to set CLOEXEC on FD.");
        close(notify_fd[0]);
        close(notify_fd[1]);
        //server_finish_process(server);
        return false;
    }

    /*
    struct wl_event_loop *loop = wl_display_get_event_loop(server->wl_display);
    server->pipe_source = wl_event_loop_add_fd(loop, notify_fd[0],
                                               WL_EVENT_READABLE, xserver_handle_ready, server);

    wl_signal_emit_mutable(&server->events.start, NULL);
*/
    xInitEventSource = Louvre::compositor()->addFdListener(notify_fd[0], this, &xserver_handle_ready);

    pid = fork();

    if (pid < 0)
    {
        LLog::error("[LXWaylandPrivate::start] fork failed. %s", strerror(errno));
        close(notify_fd[0]);
        close(notify_fd[1]);
        //server_finish_process(server);
        return false;
    }
    else if (pid == 0)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            LLog::error("[LXWaylandPrivate::start] second fork failed. %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            exec_xwayland(this, notify_fd[1]);
        }

        exit(EXIT_SUCCESS);
    }

    /* close child fds */
    /* remain managing x sockets for lazy start */
    close(notify_fd[1]);
    close(wl_fd[1]);

    if (wm_fd[1] >= 0)
        close(wm_fd[1]);

    wl_fd[1] = wm_fd[1] = -1;

    return true;
}

static bool server_start_display(LXWayland::LXWaylandPrivate *x, struct wl_display *wl_display)
{
    //server->display_destroy.notify = handle_display_destroy;
    //wl_display_add_destroy_listener(wl_display, &server->display_destroy);

    x->xDisplay = open_display_sockets(x->x_fd);
    if (x->xDisplay < 0) {
        //server_finish_display(server);
        return false;
    }

    x->xDisplayName = ":" + std::to_string(x->xDisplay);

    //snprintf(server->display_name, sizeof(server->display_name),
    //         ":%d", server->display);
    return true;
}

void LXWayland::LXWaylandPrivate::init()
{
    /*if (!getenv("WLR_XWAYLAND") && access(XWAYLAND_PATH, X_OK) != 0) {
        wlr_log(WLR_ERROR, "Cannot find Xwayland binary \"%s\"", XWAYLAND_PATH);
        return NULL;
    }*/

    if (!server_start_display(this, Louvre::compositor()->display()))
    {
    }
}
