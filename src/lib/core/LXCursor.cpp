#include <private/LXCursorPrivate.h>
#include <LTexture.h>
#include <LLog.h>
#include <X11/Xcursor/Xcursor.h>

using namespace Louvre;

LXCursor *LXCursor::loadXCursorB(const char *cursor, const char *theme, Int32 suggestedSize)
{
    XcursorImage *x11Cursor =  XcursorLibraryLoadImage(cursor, theme, suggestedSize);

    if (!x11Cursor)
    {
        LLog::error("[LXCursor::loadXCursorB()] Failed to load X Cursor.");
        return nullptr;
    }

    LXCursor *newCursor = new LXCursor();
    newCursor->imp()->hotspotB.setX((Int32)x11Cursor->xhot);
    newCursor->imp()->hotspotB.setY((Int32)x11Cursor->yhot);
    newCursor->imp()->texture = new LTexture();

    if (!newCursor->imp()->texture->setDataB(LSize((Int32)x11Cursor->width,
                                                   (Int32)x11Cursor->height),
                                                    x11Cursor->width * 4,
                                                    DRM_FORMAT_ABGR8888,
                                                    x11Cursor->pixels))
    {
        LLog::error("[LXCursor::loadXCursorB()] Failed to create texture from X Cursor.");
        delete newCursor;
        XcursorImageDestroy(x11Cursor);
        return nullptr;
    }

    XcursorImageDestroy(x11Cursor);
    return newCursor;
}

LXCursor::LXCursor()
{
    m_imp = new LXCursorPrivate();
}

LXCursor::~LXCursor()
{
    delete imp()->texture;
    delete m_imp;
}

const LTexture *LXCursor::texture() const
{
    return imp()->texture;
}

const LPoint &LXCursor::hotspotB() const
{
    return imp()->hotspotB;
}
