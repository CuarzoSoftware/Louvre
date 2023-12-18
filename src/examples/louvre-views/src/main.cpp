#include <LLauncher.h>
#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

int main(int, char *[])
{
    setenv("WAYLAND_DISPLAY", "wayland-0", 0);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);

    char *display = getenv("WAYLAND_DISPLAY");

    if (display)
        setenv("DISPLAY", display, 1);

    LLauncher::startDaemon();

    Compositor compositor;

    if (!compositor.start())
    {
        LLog::fatal("[louvre-views] Failed to start compositor.");
        return 1;
    }

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    LLog::debug("[louvre-views] Compositor uninitialized.");

    return 0;
}


