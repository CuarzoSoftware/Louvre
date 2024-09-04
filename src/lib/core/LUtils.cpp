#include "LBitset.h"
#include <LUtils.h>
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
