#include <CZ/Louvre/LClientCursor.h>
#include <CZ/Louvre/Private/LCursorPrivate.h>

using namespace Louvre;

LClientCursor::~LClientCursor() noexcept
{
    notifyDestruction();

    if (cursor()->clientCursor() == this)
        cursor()->useDefault();
}
