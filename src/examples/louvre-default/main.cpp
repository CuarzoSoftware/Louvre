#include <LCompositor.h>
#include <unistd.h>
#include <LLog.h>

using namespace Louvre;

int main(int, char *[])
{
    setenv("LOUVRE_WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    LCompositor compositor;
    // compositor.loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendX11.so");
    // compositor.loadInputBackend("/usr/etc/Louvre/backends/libLInputBackendX11.so");
    return compositor.start();
}
