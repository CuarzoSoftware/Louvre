#include <LLauncher.h>
#include <LLog.h>
#include <unistd.h>
#include "Compositor.h"

int main(int, char *[])
{
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


