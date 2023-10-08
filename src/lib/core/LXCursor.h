#ifndef LX11CURSOR_H
#define LX11CURSOR_H

#include <LNamespaces.h>

/**
 * @brief Cursor Icons and Themes
 *
 * The LXCursor class enables the search and loading of [XCursor](https://www.x.org/archive/X11R7.7/doc/man/man3/Xcursor.3.xhtml)
 * icons from installed themes on your machine.
 * It provides functionality to retrieve the icon's texture and hotspot, making it suitable for use with LCursor.
 *
 * @see LCompositor::cursorInitialized()
 */
class Louvre::LXCursor
{
public:
    /**
     * @brief Load an XCursor pixmap.
     *
     * Loads an XCursor that matches the specified name and theme.
     *
     * @param cursor Name of the XCursor to load.
     * @param theme Name of the cursor theme. Pass `NULL` if you don't want to specify a theme.
     * @param suggestedSize Suggested size of the pixmap in buffer coordinates.
     *        Returns the variant of the pixmap with closest dimensions to the specified one.
     *
     * @returns If an XCursor matching the parameters is found, returns an instance of the LXCursor class,
     *          which stores the cursor's hotspot, and texture.
     *          Otherwise, it returns `nullptr`.
     */
    static LXCursor *loadXCursorB(const char *cursor, const char *theme = NULL, Int32 suggestedSize = 64);

    /**
     * @brief Destructor of the LXCursor class.
     *
     * @warning The destructor releases any resources associated with an LXCursor instance, including its texture.
     */
    ~LXCursor();

    /// @cond OMIT
    LXCursor(const LXCursor&) = delete;
    LXCursor& operator= (const LXCursor&) = delete;
    /// @endcond

    /**
     * @brief Get the texture.
     */
    const LTexture *texture() const;

    /**
     * @brief Get the hotspot in buffer coordinates.
     */
    const LPoint &hotspotB() const;

    LPRIVATE_IMP(LXCursor)
    /// @cond OMIT
    LXCursor();
    /// @endcond
};

#endif // LX11CURSOR_H
