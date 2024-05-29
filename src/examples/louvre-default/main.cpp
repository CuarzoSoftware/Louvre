#include <LCompositor.h>
#include <LLauncher.h>
#include <LLog.h>
#include <unistd.h>
#include <LObject.h>

using namespace Louvre;

int main(int, char *[])
{
    setenv("LOUVRE_DEBUG", "3", 0);
    setenv("SRM_DEBUG", "3", 0);
    setenv("SRM_RENDER_MODE_ITSELF_FB_COUNT", "3", 0);
    setenv("SRM_FORCE_GL_ALLOCATION", "1", 0);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);
    setenv("LOUVRE_WAYLAND_DISPLAY", "wayland-2", 0);

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
