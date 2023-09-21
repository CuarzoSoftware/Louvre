#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

int main(int, char *[])
{
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);
    setenv("MESA_NO_ERROR", "1", 1);
    setenv("MESA_GLTHREAD", "1", 1);

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


