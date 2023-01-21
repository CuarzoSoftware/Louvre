#include "Compositor.h"
#include <unistd.h>

int main(int, char *[])
{
    Compositor compositor;
    compositor.start();
    return 0;
}
