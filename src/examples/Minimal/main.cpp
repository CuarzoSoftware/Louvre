#include <Compositor.h>
#include <LRect.h>

#include <fcntl.h>
#include <unistd.h>

using namespace Louvre;

int main(int, char *[])
{
    close(fileno(stdin));

    // Start the compositor
    Compositor compositor;
    compositor.start();

    return 0;
}
