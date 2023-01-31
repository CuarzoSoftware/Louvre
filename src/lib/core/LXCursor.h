#ifndef LX11CURSOR_H
#define LX11CURSOR_H

#include <LNamespaces.h>

/**
 * @brief Representation of an XCursor
 *
 * The LXCursor class represents a [XCursor](https://www.x.org/archive/X11R7.7/doc/man/man3/Xcursor.3.xhtml) loaded with the LCursor::loadXCursorB() method.
 * It provides access to its OpenGL texture and hotspot.
 */
class Louvre::LXCursor
{
public:
    LXCursor();
    ~LXCursor();

    LXCursor(const LXCursor&) = delete;
    LXCursor& operator= (const LXCursor&) = delete;

    /// Cursor texture
    LTexture *texture() const;

    /// Cursor texture size
    const LSize &sizeB() const;

    /// Hotspot
    const LPoint &hotspotB() const;

    class LXCursorPrivate;

    /*!
     * @brief Access to the private API of LXCursor.
     *
     * Returns an instance of the LXCursorPrivate class (following the ***PImpl Idiom*** pattern) which contains all the private members of LCursor.\n
     * Used internally by the library.
     */
    LXCursorPrivate *imp() const;
private:
    LXCursorPrivate *m_imp = nullptr;
};

#endif // LX11CURSOR_H
