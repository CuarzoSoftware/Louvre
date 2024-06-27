#include <LLauncher.h>
#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

int main(int, char *[])
{
    setenv("LOUVRE_DEBUG", "3", 0);
    setenv("SRM_DEBUG", "3", 0);
    setenv("SRM_RENDER_MODE_ITSELF_FB_COUNT", "2", 0);
    setenv("SRM_FORCE_GL_ALLOCATION", "1", 0);
    setenv("MOZ_ENABLE_WAYLAND", "1", 1);
    setenv("QT_QPA_PLATFORM", "wayland-egl", 1);
    setenv("LOUVRE_WAYLAND_DISPLAY", "wayland-2", 0);

    LLauncher::startDaemon();

    Compositor compositor;

    if (!compositor.start())
    {
        LLog::fatal("[louvre-weston-clone] Failed to start compositor.");
        return 1;
    }

    // Enable screencasting through xdg-desktop-portal-wlr
    LLauncher::launch("dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots | systemctl --user restart xdg-desktop-portal");

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
