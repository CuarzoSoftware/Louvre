#ifndef WAYLANDBACKENDSHARED_H
#define WAYLANDBACKENDSHARED_H

#include <CZ/skia/core/SkPoint.h>
#include <CZ/skia/core/SkSize.h>
#include <CZ/CZWeak.h>
#include <CZ/Louvre/LObject.h>
#include <wayland-client.h>
#include <mutex>
#include <sys/poll.h>
#include <vector>

using namespace Louvre;

#define LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE 64 * 64 * 4
#define LOUVRE_WAYLAND_BACKEND_CURSORS 4 // For compositors that don't release buffers immediately

struct WaylandBackendShared
{
    std::mutex mutex;
    pollfd fd[3]; // Graphic eventfd, Wayland fd, Input eventfd
    SkISize surfaceSize { 1024, 512 };
    SkISize bufferSize { 1024, 512 };
    Int32 bufferScale { 1 };

    // Cursor
    inline static wl_shm *shm { nullptr };
    inline static wl_shm_pool *shmPool { nullptr };
    inline static Int32 cursorMapFd { -1 };
    inline static UInt8 *cursorMap { nullptr };

    class SHMCursor : public LObject
    {
    public:
        SHMCursor(WaylandBackendShared &shared, std::size_t n) noexcept
        {
            buffer = wl_shm_pool_create_buffer(shmPool, n * LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE, 64, 64, 64 * 4, WL_SHM_FORMAT_ARGB8888);
            map = &shared.cursorMap[n * LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE];

            static wl_buffer_listener bufferListener { [](void *data, wl_buffer *)
            {
                static_cast<WaylandBackendShared::SHMCursor*>(data)->released = true;
            } };

            wl_buffer_add_listener(buffer, &bufferListener, this);
        }

        ~SHMCursor() noexcept
        {
            wl_buffer_destroy(buffer);
        }

        wl_buffer *buffer;
        UInt8 *map;
        bool released { true };
    };

    SHMCursor *getFreeCursor() noexcept
    {
        for (auto &cursor : cursors)
            if (cursor->released)
                return cursor.get();

        return nullptr;
    }

    std::vector<std::unique_ptr<SHMCursor>> cursors;
    CZWeak<SHMCursor> currentCursor;
    wl_surface *cursorSurface { nullptr };
    bool cursorChangedHotspot { false };
    bool cursorChangedBuffer { false };
    bool cursorVisible { false };
    SkIPoint cursorHotspot;
};

#endif // WAYLANDBACKENDSHARED_H
