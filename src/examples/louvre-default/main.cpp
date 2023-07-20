#include <LCompositor.h>
#include <unistd.h>
#include <LLog.h>

using namespace Louvre;

int main(int, char *[])
{
    /*
    setenv("LOUVRE_WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("WAYLAND_DEBUG", "1", 1);*/

    LCompositor compositor;

    if (!compositor.start())
    {
        LLog::fatal("Failed to start compositor.");
        return 1;
    }

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
