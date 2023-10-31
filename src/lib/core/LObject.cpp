#include <LCompositor.h>

using namespace Louvre;

LObject::LObject() {}

LCompositor *LObject::compositor()
{
    return LCompositor::compositor();
}

LSeat *LObject::seat()
{
    return compositor()->seat();
}

LCursor *LObject::cursor()
{
    return compositor()->cursor();
}
