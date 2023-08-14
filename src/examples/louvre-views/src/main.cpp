#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

/* TODO
 * Add non hw cursor rendering to LScene.
 * */

int main(int, char *[])
{
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores <= 0) {
        perror("Error getting number of CPU cores");
        return 1;
    }
    printf("CORES %ld", num_cores);

    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("LOUVRE_DEBUG", "4", 1);
    setenv("SRM_DEBUG", "2", 1);
    setenv("MESA_NO_ERROR", "1", 1);
    setenv("MESA_GLTHREAD", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);
    setenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", "linux-dmabuf-unstable-v1", 1);

    Compositor compositor;

    if (!compositor.start())
    {
        LLog::fatal("Failed to start compositor.");
        return 1;
    }

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}


