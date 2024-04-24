#ifndef WAYLANDBACKENDSHARED_H
#define WAYLANDBACKENDSHARED_H

#include <LPoint.h>
#include <wayland-client.h>
#include <mutex>
#include <sys/poll.h>

using namespace Louvre;

struct WaylandBackendShared
{
    std::mutex mutex;
    pollfd fd[3]; // Graphic eventfd, Wayland fd, Input eventfd
    LSize surfaceSize { 1024, 512 };
    LSize bufferSize { 1024, 512 };
    Int32 bufferScale { 1 };
    wl_buffer *cursorBuffer { nullptr };
    wl_surface *cursorSurface { nullptr };
    bool cursorChangedHotspot { false };
    bool cursorChangedBuffer { false };
    bool cursorVisible { false };
    LPoint cursorHotspot;
};

#endif // WAYLANDBACKENDSHARED_H
