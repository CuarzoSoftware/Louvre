#include <LClientCursor.h>
#include <private/LCursorPrivate.h>

using namespace Louvre;

LClientCursor::~LClientCursor() noexcept
{
    if (cursor()->clientCursor() == this)
        cursor()->useDefault();
}
