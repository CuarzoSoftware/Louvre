#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLauncher.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Ream/RLog.h>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <poll.h>
#include <mutex>
#include <cerrno>

using namespace CZ;

static CZLogger LauncherLog;

static int pipeA[2] =
{
    -1, // Daemon read end
    -1  // Compositor write end
};

static int pipeB[2] =
{
    -1, // Compositor read end
    -1  // Daemon write end
};

static pid_t daemonPID = -1;
static pid_t daemonGID = -1;

// Serializes pipe messages: commands and env updates must not interleave.
static std::mutex launchMutex;

/* Writes the whole buffer to the daemon. SIGPIPE is blocked during the write
 * so a dead daemon makes it fail with EPIPE instead of killing the compositor;
 * a SIGPIPE raised here is consumed unless one was already pending. */
static bool writeToDaemon(const char *data, size_t size)
{
    sigset_t pipeSet, prevMask, pending;
    sigemptyset(&pipeSet);
    sigaddset(&pipeSet, SIGPIPE);
    sigpending(&pending);
    const bool wasPending { sigismember(&pending, SIGPIPE) == 1 };
    pthread_sigmask(SIG_BLOCK, &pipeSet, &prevMask);

    bool ok { true };

    while (size > 0)
    {
        const ssize_t w { write(pipeA[1], data, size) };

        if (w < 0)
        {
            if (errno == EINTR)
                continue;

            ok = false;
            break;
        }

        data += w;
        size -= (size_t)w;
    }

    if (!ok && !wasPending)
    {
        const timespec zero {};
        sigtimedwait(&pipeSet, nullptr, &zero);
    }

    pthread_sigmask(SIG_SETMASK, &prevMask, nullptr);
    return ok;
}

// Reads "\x01KEY=VALUE\0" pairs until an empty line; applies each to the
// daemon environment, so system() children launched afterwards inherit it.
static bool readEnvBlock()
{
    while (true)
    {
        std::string line;
        char c = 0;

        while (true)
        {
            ssize_t n = read(pipeA[0], &c, 1);

            if (n <= 0)
                return false;

            if (c == '\0')
                break;

            line += c;
        }

        if (line.empty())
            return true;

        size_t eq = line.find('=');

        if (eq == std::string::npos || eq == 0)
            continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (value.empty())
            unsetenv(key.c_str());
        else
            setenv(key.c_str(), value.c_str(), 1);
    }
}

static Int32 daemonLoop()
{
    close(pipeA[1]);
    close(pipeB[0]);

    fcntl(pipeA[0], F_SETFD, fcntl(pipeA[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipeB[1], F_SETFD, fcntl(pipeB[1], F_GETFD) | FD_CLOEXEC);

    if (setpgid(0, 0) == 0)
        daemonGID = getpgrp();

    pollfd fds;
    fds.events = POLLIN;
    fds.revents = 0;
    fds.fd = pipeA[0];

    // Read messages from the parent
    std::string cmd = "";
    char c;

    while (true)
    {
        if (poll(&fds, 1, -1) != 1)
            return 1;

        Int32 n = read(pipeA[0], &c, sizeof(c));

        // Closed pipe
        if (n == 0)
            return 0;

        while (n > 0)
        {
            // Env update block
            if (c == '\x01')
            {
                if (!readEnvBlock())
                    return 0;

                cmd = "";
                break;
            }

            // Reached end of command
            if (c == '\0')
            {
                pid_t pid = fork();

                if (pid == 0)
                    exit(system(cmd.c_str()));
                else if (pid > 0)
                {
                    // Send the launched app PID to the compositor
                    char buffer[32];
                    sprintf(buffer, "%d", pid);
                    char *ptr = buffer;

                    while (true)
                    {
                        ssize_t w = write(pipeB[1], ptr, sizeof(c));

                        if (*ptr == '\0')
                            break;

                        if (w > 0)
                            ptr += w;
                    }
                }

                cmd = "";
                break;
            }
            else
            {
                // Append character to the command
                cmd += c;
            }

            n = read(pipeA[0], &c, sizeof(c));
        }
    }

    return 1;
}

pid_t LLauncher::startDaemon(const std::string &name)
{
    LauncherLog = LLog.newWithContext("LLauncher");

    if (daemonPID != -1)
    {
        LauncherLog(CZError, CZLN, "Failed to start daemon, already running");
        goto error;
    }

    if (compositor())
    {
        LauncherLog(CZError, CZLN, "Failed to start daemon. Must be launched before an LCompositor instance is created");
        goto error;
    }

    if (pipe(pipeA) != 0)
    {
        LauncherLog(CZError, CZLN, "Failed to start daemon. Failed to create pipe: {}", strerror(errno));
        goto error;
    }

    if (pipe(pipeB) != 0)
    {
        LauncherLog(CZError, CZLN, "Failed to start daemon. Failed to create pipe: {}", strerror(errno));
        goto closePipeA;
    }

    daemonGID = -1;
    daemonPID = fork();

    if (daemonPID == -1)
    {
        LauncherLog(CZError, CZLN, "Failed to start daemon. Failed to create daemon fork: {}", strerror(errno));
        goto closePipeB;
    }
    // Child
    else if (daemonPID == 0)
    {
        setenv("CZ_REAM_LOG_LEVEL", "2", 1); // Launched apps: Ream errors only, even if the compositor logs more
        char *display = getenv("CZ_LOUVRE_WAYLAND_DISPLAY");

        if (display)
            setenv("WAYLAND_DISPLAY", display , 1);

        for (Int32 x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
            if (x != pipeA[0] && x != pipeB[1] && x != STDOUT_FILENO && x != STDERR_FILENO)
                close(x);

        Int32 nullFD = open("/dev/null", O_RDONLY);

        if (nullFD != -1)
            dup2(nullFD, STDIN_FILENO);

        prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
        Int32 ret = daemonLoop();
        LauncherLog(CZDebug, "[{}] Daemon exited with status {}", name.c_str(), ret);

        close(pipeA[0]);
        close(pipeB[1]);

        if (nullFD != 1)
            close(nullFD);

        if (daemonGID != -1)
            kill(-daemonGID , SIGTERM);

        exit(ret);
    }
    else
    {
        close(pipeA[0]);
        close(pipeB[1]);
        LauncherLog(CZDebug, "LLauncher daemon started successfully with PID: {}", daemonPID);
        return daemonPID;
    }

closePipeB:
    close(pipeB[0]);
    close(pipeB[1]);
closePipeA:
    close(pipeA[0]);
    close(pipeA[1]);
error:
    return -1;
}

pid_t LLauncher::pid()
{
    return daemonPID;
}

pid_t LLauncher::launch(const std::string &command)
{
    if (daemonPID < 0)
    {
        LauncherLog(CZError, CZLN, "Cannot launch {}. Daemon is not running", command.c_str());
        return -1;
    }

    if (command.empty())
    {
        LauncherLog(CZError, CZLN, "Cannot launch {}. Invalid command", command.c_str());
        return -1;
    }

    std::lock_guard<std::mutex> lock(launchMutex);

    pollfd fds;
    fds.events = POLLIN;
    fds.revents = 0;
    fds.fd = pipeB[0];
    char c;

    /* Discard a late reply from a previous launch() that timed out. Poll before
     * every read: the pipe is blocking, so reading past the available bytes
     * would hang the compositor. */
    while (poll(&fds, 1, 0) == 1 && read(pipeB[0], &c, 1) > 0) {}

    ssize_t s;
    std::string res = "";

    // Send the command, including its null terminator
    if (!writeToDaemon(command.c_str(), command.size() + 1))
    {
        LauncherLog(CZError, "Command {} failed. Daemon died", command.c_str());
        stopDaemon();
        return -1;
    }

    // the launched app PID
    while (true)
    {
        if (poll(&fds, 1, 1000) != 1)
            return -1;

        s = read(pipeB[0], &c, 1);

        if (s <= 0)
        {
            LauncherLog(CZError, "Command {} failed. Daemon died", command.c_str());
            stopDaemon();
            return -1;
        }

        if (c == '\0')
        {
            pid_t pid = atoi(res.c_str());

            if (pid > 0)
                LauncherLog(CZInfo, "Command {} executed successfuly. PID: {}", command.c_str(), pid);
            else
                LauncherLog(CZError, CZLN, "Command {} failed. PID: {}", command.c_str(), pid);

            return pid;
        }

        res += c;
    }

    return -1;
}

bool LLauncher::setEnv(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(launchMutex);

    if (daemonPID < 0)
    {
        LauncherLog(CZError, CZLN, "Cannot set env {}. Daemon is not running", key);
        return false;
    }

    // '\0' would split the message (the daemon reads null-terminated pairs)
    if (key.empty() || key.find('=') != std::string::npos || key.find('\0') != std::string::npos)
    {
        LauncherLog(CZError, CZLN, "Cannot set env {}. Invalid variable name", key);
        return false;
    }

    if (value.find('\0') != std::string::npos)
    {
        LauncherLog(CZError, CZLN, "Cannot set env {}. The value contains a null character", key);
        return false;
    }

    // \x01 tags an env update; the trailing empty line closes the block.
    std::string msg = std::string(1, '\x01') + key + "=" + value;
    msg.push_back('\0');
    msg.push_back('\0');

    if (!writeToDaemon(msg.data(), msg.size()))
    {
        LauncherLog(CZError, CZLN, "Failed to send the env update {}. Daemon died", key);
        stopDaemon();
        return false;
    }

    return true;
}

void LLauncher::stopDaemon()
{
    if (daemonPID < 0)
        return;

    daemonPID = -1;

    close(pipeB[0]);
    close(pipeA[1]);

    LauncherLog(CZInfo, "Daemon stopped");
}
