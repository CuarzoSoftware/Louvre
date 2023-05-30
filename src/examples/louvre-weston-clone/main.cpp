#include "Compositor.h"
#include <LLog.h>
#include <unistd.h>

int main(int, char *[])
{
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
