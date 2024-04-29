#ifndef LX11CURSOR_H
#define LX11CURSOR_H

#include <LTexture.h>

/**
 * @brief XCursor Icons and Themes
 *
 * The LXCursor class enables the search and loading of [XCursor](https://www.x.org/archive/X11R7.7/doc/man/man3/Xcursor.3.xhtml)
 * icons from installed themes on your machine.\n
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
     * @param suggestedSize Suggested buffer size (width or height) of the pixmap.
     *        Returns the variant of the pixmap with closest dimensions to the specified one.
     *
     * @returns If an XCursor matching the parameters is found, returns an instance of the LXCursor class,
     *          which stores the cursor's hotspot, and texture.
     *          Otherwise, it returns `nullptr`.
     */
    static LXCursor *load(const char *cursor, const char *theme = NULL, Int32 suggestedSize = 64) noexcept;

    /**
     * @brief Destructor of the LXCursor class.
     *
     * @warning The destructor releases any resources associated with an LXCursor instance, including its texture.
     */
    ~LXCursor() = default;

    LCLASS_NO_COPY(LXCursor)

    /**
     * @brief Get the cursor's texture.
     */
    const LTexture *texture() const noexcept
    {
        return &m_texture;
    }

    /**
     * @brief Get the cursor's hotspot in buffer coordinates.
     */
    const LPoint &hotspotB() const noexcept
    {
        return m_hotspotB;
    }

private:
    LXCursor() noexcept = default;
    LTexture m_texture;
    LPoint m_hotspotB;
};

#endif // LX11CURSOR_H
