#include <LNamespaces.h>
#include <LUtils.h>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static void randname(char *buffer)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    for (int i = 0; i < 6; ++i)
    {
        buffer[i] = 'A'+(r&15)+(r&16)*2;
        r >>= 5;
    }
}

static int anonymousSHMOpen()
{
    char name[] { "/louvre-wayland-XXXXXX" };
    int retries = 100;

    do
    {
        randname(name + strlen(name) - 6);
        --retries;

        int fd { shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600) };
        if (fd >= 0)
        {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);

    return -1;
}

int Louvre::createSHM(std::size_t size)
{
    int fd { anonymousSHMOpen() };

    if (fd < 0)
        return fd;

    if (ftruncate(fd, size) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

bool Louvre::setCloexec(int fd, bool cloexec) noexcept
{
    Int32 flags = fcntl(fd, F_GETFD);

    if (flags == -1)
        return false;

    if (cloexec)
        flags = flags | FD_CLOEXEC;
    else
        flags = flags & ~FD_CLOEXEC;

    if (fcntl(fd, F_SETFD, flags) == -1)
        return false;

    return true;
}

std::vector<std::string> Louvre::splitString(const std::string &str, char delimiter) noexcept
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

bool Louvre::isExecutable(const std::filesystem::path &path) noexcept
{
    return access(path.c_str(), X_OK) == 0;
}

std::filesystem::path Louvre::whereIsExecutable(const std::string &exeName) noexcept
{
    const std::string pathEnv { getenvString("PATH") };

    if (pathEnv.empty())
        return std::filesystem::path();

    const std::vector<std::string> directories { splitString(pathEnv, ':') };

    std::filesystem::path fullPath;

    for (const auto &dir : directories)
    {
        fullPath = std::filesystem::path(dir) / exeName;

        if (isExecutable(fullPath))
            return fullPath;
    }

    return std::filesystem::path();
}
