#include <LLog.h>
#include <LXCursor.h>
#include <X11/Xcursor/Xcursor.h>

using namespace Louvre;

LXCursor *LXCursor::load(const char *cursor, const char *theme,
                         Int32 suggestedSize) noexcept {
  XcursorImage *x11Cursor =
      XcursorLibraryLoadImage(cursor, theme, suggestedSize);

  if (!x11Cursor) {
    LLog::error("[LXCursor::loadXCursorB] Failed to load X Cursor.");
    return nullptr;
  }

  LXCursor *newCursor = new LXCursor();
  newCursor->m_hotspotB.setX((Int32)x11Cursor->xhot);
  newCursor->m_hotspotB.setY((Int32)x11Cursor->yhot);

  if (!newCursor->m_texture.setDataFromMainMemory(
          LSize((Int32)x11Cursor->width, (Int32)x11Cursor->height),
          x11Cursor->width * 4, DRM_FORMAT_ABGR8888, x11Cursor->pixels)) {
    LLog::error(
        "[LXCursor::loadXCursorB] Failed to create texture from X Cursor.");
    delete newCursor;
    XcursorImageDestroy(x11Cursor);
    return nullptr;
  }

  XcursorImageDestroy(x11Cursor);
  return newCursor;
}
