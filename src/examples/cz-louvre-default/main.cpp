#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLauncher.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

int main(int, char *[])
{
    /* setenv("XKB_DEFAULT_LAYOUT", "latam", 0) */

    setenv("CZ_LOUVRE_WAYLAND_DISPLAY", "louvre", 0);
    setenv("CZ_LOUVRE_ENABLE_LIBSEAT",  "1", 0);
    setenv("CZ_CORE_LOG_LEVEL",         "4", 0);
    setenv("CZ_REAM_LOG_LEVEL",         "4", 0);
    setenv("CZ_SRM_LOG_LEVEL",          "4", 0);
    setenv("CZ_LOUVRE_LOG_LEVEL",       "4", 0);

    LLauncher::startDaemon();

    LCompositor compositor;

    if (!compositor.start())
    {
        LLog(CZFatal, CZLN, "Failed to start compositor");
        return 1;
    }

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.dispatch(-1);

    return 0;
}
