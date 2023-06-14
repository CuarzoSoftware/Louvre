#include <private/LXCursorPrivate.h>
#include <LTexture.h>
#include <X11/Xcursor/Xcursor.h>

using namespace Louvre;

LXCursor *LXCursor::loadXCursorB(const char *cursor, const char *theme, Int32 suggestedSize, GLuint textureUnit)
{
    XcursorImage *x11Cursor =  XcursorLibraryLoadImage(cursor, theme, suggestedSize);

    if (!x11Cursor)
        return nullptr;

    LXCursor *newCursor = new LXCursor();
    newCursor->imp()->hotspotB.setX(x11Cursor->xhot);
    newCursor->imp()->hotspotB.setY(x11Cursor->yhot);
    newCursor->imp()->texture = new LTexture(textureUnit);
    newCursor->imp()->texture->setDataB(LSize((Int32)x11Cursor->width, (Int32)x11Cursor->height),
                                        x11Cursor->width * 4,
                                        DRM_FORMAT_ABGR8888,
                                        x11Cursor->pixels);
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

const LSize &LXCursor::sizeB() const
{
    return imp()->texture->sizeB();
}

const LPoint &LXCursor::hotspotB() const
{
    return imp()->hotspotB;
}
