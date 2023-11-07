#ifndef LCURSOR_H
#define LCURSOR_H

#include <LObject.h>
#include <LPoint.h>
#include <LTexture.h>
#include <LRect.h>

/**
 * @brief Utility class for rendering cursors.
 *
 * The LCursor class is designed to make cursor rendering easier and take advantage of compositing properties of
 * certain graphic backends to improve performance.\n
 *
 * @subsection hw_composition Hardware Composition
 *
 * Some graphic backends, such as DRM, allow for hardware cursor compositing, which can improve performance by reducing the need to repaint
 * an output every time the cursor changes position.\n
 * To check if an output supports hardware cursor compositing, use the hasHardwareSupport() method.\n
 * If hardware compositing is not supported, the cursor needs to be rendered using OpenGL.
 * In that case, the cursor's position (with the hotspot offset included) and size can be accessed using the rect() method, and its current texture with the texture() method.
 *
 * @subsection performance Smoother Hardware Cursor Updates
 *
 * By default, the DRM graphic backend uses the Atomic DRM API, which synchronizes hardware cursor updates with the refresh rate of the outputs.
 * However, this synchronization can result in laggy cursor updates when rendering take a long time.
 * On the other hand, the legacy DRM API supports asynchronous hardware cursor updates, which can provide a smoother cursor experience.
 * To switch to the legacy DRM API, set the **SRM_FORCE_LEGACY_API** environment variable to 1.
 *
 * @warning Please note that using the legacy DRM API may cause issues with certain proprietary Nvidia drivers.
*/
class Louvre::LCursor : public LObject
{
public:

    /// @cond OMIT
    LCursor(const LCursor&) = delete;
    LCursor& operator= (const LCursor&) = delete;
    /// @endcond

    /**
     * @brief Load the default cursor.
     *
     * This method sets the cursor's texture and hotspot to the default values
     * configured using replaceDefaultB().
     *
     * The default texture initially matches defaultLouvreTexture() with a hotspot at (8, 8).
     */
    void useDefault();

    /**
     * @brief Replace Louvre's default cursor.
     *
     * This method allows you to replace the Louvre's default cursor, which is set when
     * useDefault() is called. You can specify a custom texture and hotspot for
     * the new cursor. To restore Louvre's default cursor, pass `nullptr` as the texture.
     *
     * @param texture The new texture to use as the default cursor, or `nullptr` to
     * restore the default Louvre cursor.
     *
     * @param hotspot The hotspot position for the new cursor in buffer coordinates.
     *
     */
    void replaceDefaultB(const LTexture *texture, const LPointF &hotspot);

    /**
     * @brief Set the cursor texture.
     *
     * Assigns the texture and hotspot of the cursor. The texture size does not necessarily define the cursor size, setSize() must be used to assign the cursor size.\n
     *
     * @param texture Texture to assign.
     * @param hotspot Cursor hotspot in buffer coordinates.
     */
    void setTextureB(const LTexture *texture, const LPointF &hotspot);

    /**
     * @brief Get the current cursor texture.
     */
    LTexture *texture() const;

    /**
     * @brief Get the default cursor texture.
     *
     * This method returns the texture that has been set using replaceDefaultB().
     * Initially, the default texture is the same as defaultLouvreTexture().
     *
     * @return A pointer to the default cursor texture.
     */
    LTexture *defaultTexture() const;

    /**
     * @brief Get the default cursor hotspot.
     *
     * This method returns the hotspot that has been set using replaceDefaultB().
     * Initially, the default hotspot is (8, 8).
     *
     * @return A constant reference to the default cursor hotspot.
     */
    const LPointF &defaultHotspotB() const;

    /**
     * @brief Default Louvre's cursor texture.
     *
     * <center><IMG SRC="https://lh3.googleusercontent.com/MSUUg3LSS6lYtpyLnKzbECf9eeZeFscmnLGJLRCdADwcjjcVd4xT07AMvQoHUTGptJFzY4tZrQ3IdLKyEbM_O0WyWYk8Pvc-Jf8xZHXoFUkFo2RRYTP8zN_LeOhsvIc6SlsO83TJUw=w2400"></center>
     *
     * The default Louvre's cursor is the one shown in the image, with a size of 64x64 pixels and hotspot at (8,8).
     */
    LTexture *defaultLouvreTexture() const;

    /**
     * @brief Move the cursor.
     *
     * Adjusts the cursor position by a delta (dx, dy) in surface coordinates.
     *
     * @param dx Delta x in surface coordinates.
     * @param dy Delta y in surface coordinates.
     *
     * @note Louvre automatically repositions the cursor if the new position is not within any output.
     */
    void move(Float32 dx, Float32 dy);

    /**
     * @brief Set the cursor position.
     *
     * Sets the cursor position in surface coordinates.
     *
     * @param pos The desired cursor position.
     *
     * @note Louvre automatically repositions the cursor if the specified position is not within any output.
     */
    void setPos(const LPointF &pos);

    /**
     * @brief Set the cursor position.
     *
     * Sets the cursor position in surface coordinates.
     *
     * @param x The desired x cursor position.
     * @param y The desired y cursor position.
     *
     * @note Louvre automatically repositions the cursor if the specified position is not within any output.
     */
    void setPos(Float32 x, Float32 y);

    /**
     * @brief Get the current cursor position.
     *
     * Returns the current cursor position.
     */
    const LPointF &pos() const;

    /**
     * @brief Get the cursor rect on the screen.
     *
     * Returns the cursor rect, which is defined as LRect(pos - hotspot, size), in surface coordinates.
     *
     * Use this rect and texture() to paint the cursor with LPainter when hardware composition is not supported.
     */
    const LRect &rect() const;

    /**
     * @brief Set the cursor hotspot in buffer coordinates.
     *
     * The cursor hotspot is defined by coordinates relative to the origin of its buffer (upper left corner) and is used to position it correctly on the screen.
     * For example, if the texture has a size of (64, 64), and the hotspot is at the center (32, 32), then the cursor's final screen position would be (x - 32, y - 32)
     * for a cursor at position (x, y).
     *
     * @note The hotspot is automatically scaled proportionally to the cursor size.
     *
     * @param hotspot The desired hotspot in buffer coordinates.
     */
    void setHotspotB(const LPointF &hotspot);

    /**
     * @brief Get the current cursor hotspot in buffer coordinates.
     */
    const LPointF &hotspotB() const;

    /**
     * @brief Set the cursor size.
     *
     * Sets the cursor size using surface coordinates. The texture and hotspot are automatically scaled,
     * with the hotspot maintaining its proportion to the texture buffer size.
     *
     * @note You don't need to set the cursor size every time you change its texture or hotspot, Louvre automatically updates it.
     *
     * The initial cursor size is (24, 24).
     *
     * @param size The desired cursor size in surface coordinates.
     */
    void setSize(const LSizeF &size);

    /**
     * @brief Change cursor visibility.
     *
     * Shows or hides the cursor. If an output does not support hardware composition it is your responsibility to avoid rendering it when it is hidden.\n
     * You can use the visible() property to know if it is visible.
     *
     * @param state `true` makes the cursor visible and `false` hides it.
     */
    void setVisible(bool state);

    /**
     * @brief Indicates whether the cursor is visible.
     *
     * @returns `true` if visible and `false` if hidden.
     */
    bool visible() const;

    /**
     * @brief Repaint intersected outputs.
     *
     * Invokes LOutput::repaint() for each output in the list of intersected outputs.
     *
     * @param nonHardwareOnly If true, only repaints outputs that do not support hardware compositing.
     */
    void repaintOutputs(bool nonHardwareOnly = true);

    /**
     * @brief Check for hardware compositing support.
     *
     * Indicates whether the specified output supports hardware compositing.
     *
     * @return `true` if hardware compositing is supported and `false` otherwise.
     */
    bool hasHardwareSupport(const LOutput *output) const;

    /**
     * @brief Get the current cursor output.
     *
     * Returns a pointer to the output where the cursor is currently positioned.
     *
     * @note Louvre guarantees that the cursor is always within an output.
     */
    LOutput *output() const;

    /**
     * @brief Get a list of intersected outputs.
     *
     * Returns a list of initialized outputs that intersect with the cursor's rect() property.
     */
    const std::list<LOutput*> &intersectedOutputs() const;

    LPRIVATE_IMP(LCursor)

    /// @cond OMIT
    friend class Louvre::LCompositor;
    friend class Louvre::LOutput;

    /**
     * @brief Constructor of the LCursor class.
     * @param output The output on which the cursor is initialized.
     */
    LCursor();

    /**
     * @brief Desctructor of the LCursor class.
     */
    ~LCursor();
    /// @endcond
};

#endif // LCURSOR_H
