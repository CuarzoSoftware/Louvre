#include "Compositor.h"

int main(int, char *[])
{

    // Start the compositor
    Compositor compositor;
    compositor.loadGraphicBackend("/usr/local/lib/Louvre/backends/libLBackendDRM.so");
    compositor.start();

    return 0;
}
