#ifndef LX11CURSOR_H
#define LX11CURSOR_H

#include <LNamespaces.h>

/**
 * @brief Representation of an XCursor
 *
 * The LXCursor class represents an [XCursor](https://www.x.org/archive/X11R7.7/doc/man/man3/Xcursor.3.xhtml) loaded using the loadXCursorB() method.
 * It offers access to its OpenGL texture and hotspot.
 */
class Louvre::LXCursor
{
public:
    /*!
     * @brief Loads an XCursor pixmap.
     *
     * Loads pixmaps of X cursors available in the system and converts them to a texture.
     *
     * @param cursor Name of the XCursor to load.
     * @param theme Name of the cursor theme. You can enter `NULL` if you don't want to specify a theme.
     * @param suggestedSize Suggested size of the pixmap in buffer coordinates. Returns the variant of the pixmap with closest dimensions to the specified one.
     *
     * @returns If an XCursor matching the parameters is found, returns an instance of the LXCursor class, which stores the cursor's dimensions, hotspot, and texture. Otherwise, it returns `nullptr`. See the example available in LCompositor::cursorInitialized() for more information.
     */
    static LXCursor *loadXCursorB(const char *cursor, const char *theme = NULL, Int32 suggestedSize = 64);

    /*!
     * @brief Destructor for LXCursor.
     *
     * @warning The destructor releases any resources associated with an LXCursor instance, including its texture.
     */
    ~LXCursor();

    /// @cond OMIT
    LXCursor(const LXCursor&) = delete;
    LXCursor& operator= (const LXCursor&) = delete;
    /// @endcond

    /*!
     * @brief Get the cursor texture.
     *
     * @return A pointer to the cursor's texture.
     */
    const LTexture* texture() const;

    /*!
     * @brief Get the cursor texture size in buffer coordinates.
     *
     * @return The size of the cursor's texture.
     */
    const LSize& sizeB() const;

    /*!
     * @brief Get the cursor hotspot in buffer coordinates.
     *
     * @return The hotspot of the cursor.
     */
    const LPoint& hotspotB() const;

    LPRIVATE_IMP(LXCursor)
    /// @cond OMIT
    LXCursor();
    /// @endcond
};

#endif // LX11CURSOR_H
