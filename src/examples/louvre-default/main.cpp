#include <LCompositor.h>
#include <unistd.h>
#include <LLog.h>

using namespace Louvre;

int main(int, char *[])
{

    LCompositor compositor;

    /* Cargar backends específicos
     *
     * compositor.loadGraphicBackend("/usr/etc/Louvre/backends/libLGraphicBackendX11.so");
     * compositor.loadInputBackend("/usr/etc/Louvre/backends/libLInputBackendX11.so");
     */

    return compositor.start();
}
