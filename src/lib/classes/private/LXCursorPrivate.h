#ifndef LX11CURSORPRIVATE_H
#define LX11CURSORPRIVATE_H

#include <LXCursor.h>
#include <LPoint.h>

class Louvre::LXCursor::LXCursorPrivate
{
public:
    LXCursorPrivate()                                         = default;
    ~LXCursorPrivate()                                        = default;

    LXCursorPrivate(const LXCursorPrivate&)                 = delete;
    LXCursorPrivate& operator= (const LXCursorPrivate&)     = delete;

    LPoint hotspotB;
    LTexture *texture;

};

#endif // LX11CURSORPRIVATE_H
