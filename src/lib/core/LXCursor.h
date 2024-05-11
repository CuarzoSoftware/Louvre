#ifndef LX11CURSOR_H
#define LX11CURSOR_H

#include <LTexture.h>

/**
 * @brief An XCursor icon.
 *
 * XCursor icons are widely used for cursor themes in Linux.\n
 * This class facilitates loading icons installed on the system and retrieving
 * their hotspot and texture, which can then be applied to LCursor.
 *
 * Example usage:
 * @code
 *  LXCursor *handCursor { LXCursor::load("hand2", "Adwaita", 64) };
 *
 *  if (handCursor)
 *      cursor()->setTextureB(handCursor->texture(),
 *                            handCursor->hotspotB());
 * @endcode
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
     * @brief Destructor
     *
     * Release the icon resources, including the texture.
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
    LTexture m_texture { true };
    LPoint m_hotspotB;
};

#endif // LX11CURSOR_H
