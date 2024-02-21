#include <LCompositor.h>
#include <LLauncher.h>
#include <LLog.h>
#include <unistd.h>
#include <LObject.h>

using namespace Louvre;

int main(int, char *[])
{
    setenv("LOUVRE_DEBUG", "1", 0);
    setenv("SRM_DEBUG", "1", 0);
    setenv("WAYLAND_DISPLAY", "wayland-0", 0);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);

    char *display = getenv("WAYLAND_DISPLAY");

    if (display)
        setenv("DISPLAY", display, 1);

    LLauncher::startDaemon();

    LCompositor compositor;

    if (!compositor.start())
    {
        LLog::fatal("[louvre-default] Failed to start compositor.");
        return 1;
    }

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
