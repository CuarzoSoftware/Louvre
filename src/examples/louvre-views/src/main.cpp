#include <LLauncher.h>
#include <LLog.h>
#include <unistd.h>
#include "Compositor.h"

int main(int, char *[])
{
    setenv("LOUVRE_DEBUG", "1", 0);
    setenv("SRM_DEBUG", "1", 0);
    setenv("SRM_RENDER_MODE_ITSELF_FB_COUNT", "3", 0);

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
