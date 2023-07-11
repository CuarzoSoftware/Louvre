#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

int main(int, char *[])
{
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    //setenv("LOUVRE_DEBUG", "4", 1);
    setenv("SRM_DEBUG", "1", 1);

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


