#ifndef LX11CURSORPRIVATE_H
#define LX11CURSORPRIVATE_H

#include <LXCursor.h>
#include <LPoint.h>

using namespace Louvre;

LPRIVATE_CLASS(LXCursor)
    LPoint hotspotB;
    LTexture *texture;
};

#endif // LX11CURSORPRIVATE_H
