#include "Shared.h"
#include "Compositor.h"

Compositor *comp()
{
    return (Compositor*)LCompositor::compositor();
}

std::list<Output *> &outps()
{
    return (std::list<Output*>&)comp()->outputs();
}

Int32 minimizedItemHeight()
{
    return 48*comp()->globalScale();
}
