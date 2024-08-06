#ifndef LCURSOR_H
#define LCURSOR_H

#include <LObject.h>
#include <LPoint.h>
#include <LWeak.h>
#include <memory>

/**
 * @brief Utility class for rendering cursors.
 *
 * @anchor lcursor_detailed
 *
 * The LCursor class is designed to make cursor rendering easier and take advantage of compositing properties of
 * certain graphic backends to improve performance.
 *
 * @see LXCursor, LClientCursor, LCursorRole.
 *
 * @note Clients can request to set the cursor through LPointer::setCursorRequest(), see LClientCursor.
 *
 * @subsection hw_composition Hardware Composition
 *
 * Some graphic backends, such as DRM, allow for hardware cursor compositing, which can improve performance by reducing the need to repaint
 * an output every time the cursor changes position.\n
 * To check if an output supports hardware cursor compositing, use hasHardwareSupport().\n
 * If hardware compositing is not supported for a specific output, the cursor is rendered by Louvre using OpenGL after a paintGL() event (when needed).
 * In such cases the damage region provided by damage() has to be repainted.
 */
class Louvre::LCursor final : public LObject
{
public:

    LCLASS_NO_COPY(LCursor)

    /**
     * @brief Sets the cursor position.
     *
     * Sets the cursor position in compositor-global coordinates.
     *
     * @param pos The new cursor position.
     *
     * @note Louvre automatically repositions the cursor if the specified position is not within any output.
     */
    void setPos(const LPointF &pos) noexcept;

    /**
     * @brief Set the cursor position.
     *
     * @see setPos()
     */
    void setPos(Float32 x, Float32 y) noexcept;

    /**
     * @brief Gets the current cursor position in compositor-global coordinates.
     */
    const LPointF &pos() const noexcept
    {
        return m_pos;
    }

    /**
     * @brief Moves the cursor.
     *
     * Adjusts the cursor position by a delta (dx, dy) in surface coordinates.
     *
     * @param dx Delta x in surface coordinates.
     * @param dy Delta y in surface coordinates.
     *
     * @note Louvre automatically repositions the cursor if the new position is not within any output.
     */
    void move(Float32 dx, Float32 dy) noexcept;

    /**
     * @brief Moves the cursor.
     *
     * @see move()
     */
    void move(const LPointF &delta) noexcept;

    /**
     * @brief Gets the cursor rect on the screen.
     *
     * Returns the cursor rect, which is defined as LRect(pos - hotspot, size), in compositor-global coordinates.
     *
     * You can use this rect and texture() to paint the cursor with LPainter if needed.
     */
    const LRect &rect() const noexcept;

    /**
     * @brief Sets the cursor size.
     *
     * Sets the cursor size in surface coordinates. The texture and hotspot are automatically scaled,
     * with the hotspot maintaining its proportion to the texture buffer size.
     *
     * @note You don't need to set the cursor size every time you change its texture or hotspot, Louvre automatically updates it.
     *
     * The initial cursor size is (24, 24).
     *
     * @param size The desired cursor size in surface coordinates.
     */
    void setSize(const LSizeF &size) noexcept;

    /**
     * @brief Toggles the cursor visibility.
     *
     * @param state `true` to set visible, `false` otherwise.
     */
    void setVisible(bool state) noexcept;

    /**
     * @brief Checks if the cursor is visible.
     *
     * @see setVisible()
     */
    bool visible() const noexcept;

    /**
     * @brief Assigns an LClientCursor.
     *
     * While an LClientCursor is assigned, the cursor is automatically updated when the client
     * updates its LCursorRole surface texture, hotspot, and visibility.
     *
     * If the LClientCursor is destroyed while set, useDefault() is called and the cursor is set to visible.
     *
     * @see LPointer::setCursorRequest()
     * @see LClient::lastCursorRequest()
     *
     * @param clientCursor The LClientCursor to assign.
     */
    void setCursor(const LClientCursor &clientCursor) noexcept;

    /**
     * @brief Assigns an LXCursor.
     *
     * @note Passing `nullptr` is a no-op.
     */
    void setCursor(const LXCursor *xcursor) noexcept;

    /**
     * @brief Retrieves the client cursor set with setCursor().
     *
     * @return A pointer to the LClientCursor if set, `nullptr` otherwise.
     */
    const LClientCursor *clientCursor() const noexcept;

    /**
     * @brief Gets the current cursor texture.
     *
     * @return A pointer to the current texture, `nullptr` otherwise.
     */
    LTexture *texture() const noexcept;

    /**
     * @brief Sets the cursor texture.
     *
     * Assigns the texture and hotspot of the cursor. The texture size does not necessarily define the cursor size, see setSize().
     *
     * @param texture Texture to assign.
     * @param hotspot Cursor hotspot in buffer coordinates.
     */
    void setTextureB(const LTexture *texture, const LPointF &hotspot) noexcept;

    /**
     * @brief Restores the default cursor.
     *
     * Sets the cursor's texture and hotspot to the default values
     * configured using replaceDefaultB().
     *
     * The default texture initially matches defaultLouvreTexture() with a hotspot at (8, 8).
     */
    void useDefault() noexcept;

    /**
     * @brief Replaces Louvre's default cursor.
     *
     * This method allows you to replace the Louvre's default cursor texture and hotspot, which is set when
     * useDefault() is called.
     *
     * @param texture The new texture to use as the default cursor, or `nullptr` to
     * restore the default Louvre cursor.
     *
     * @param hotspot The hotspot position for the new cursor in buffer coordinates.
     */
    void replaceDefaultB(const LTexture *texture, const LPointF &hotspot) noexcept;

    /**
     * @brief Gets the default cursor texture.
     *
     * This method returns the texture that has been set using replaceDefaultB().\n
     * Initially, the default texture is the same as defaultLouvreTexture().
     *
     * @return A pointer to the default cursor texture.
     */
    LTexture *defaultTexture() const noexcept;

    /**
     * @brief Gets the default cursor hotspot.
     *
     * The hotspot that has been set using replaceDefaultB().\n
     * Initially set to (8, 8).
     */
    const LPointF &defaultHotspotB() const noexcept;

    /**
     * @brief Default Louvre's cursor texture.
     *
     * <center><IMG SRC="https://lh3.googleusercontent.com/MSUUg3LSS6lYtpyLnKzbECf9eeZeFscmnLGJLRCdADwcjjcVd4xT07AMvQoHUTGptJFzY4tZrQ3IdLKyEbM_O0WyWYk8Pvc-Jf8xZHXoFUkFo2RRYTP8zN_LeOhsvIc6SlsO83TJUw=w2400"></center>
     *
     * The default Louvre's cursor has a size of 64x64 pixels and hotspot at (8,8).
     */
    LTexture *defaultLouvreTexture() const noexcept;

    /**
     * @brief Sets the cursor hotspot in buffer coordinates.
     *
     * The cursor hotspot is defined by coordinates relative to the origin of its buffer (upper left corner).
     *
     * @note The hotspot is automatically scaled proportionally to the cursor size().
     *
     * @param hotspot The desired hotspot in buffer coordinates.
     */
    void setHotspotB(const LPointF &hotspot) noexcept;

    /**
     * @brief Gets the current cursor hotspot in buffer coordinates.
     */
    const LPointF &hotspotB() const noexcept;

    /**
     * @brief Enables or disables LCursor for the specified output.
     *
     * By default, LCursor is enabled for all outputs.
     *
     * @param output The output for which to enable or disable LCursor.
     * @param enable Set to `true` to enable LCursor, or `false` to disable it.
     */
    void enable(LOutput *output, bool enable) noexcept;

    /**
     * @brief Checks if LCursor is enabled for the specified output.
     *
     * @see enable()
     *
     * @param output The output to check.
     * @return `true` if LCursor is enabled for the given output, `false` otherwise.
     */
    bool enabled(LOutput *output) const noexcept;

    /**
     * @brief Checks if a given output supports hardware compositing.
     *
     * @return `true` if hardware compositing is supported and `false` otherwise.
     */
    bool hasHardwareSupport(const LOutput *output) const noexcept;

    /**
     * @brief Toggles hardware compositing for the specified output.
     *
     * When disabled, Louvre will render the cursor using OpenGL after an LOutput::paintGL() event.
     *
     * @see damage()
     *
     * Hardware compositing is enabled by default if the output supports it, see hasHardwareSupport().
     *
     * @param output The output for which to enable or disable hardware compositing.
     * @param enabled Set to `true` to enable hardware compositing, or `false` to disable it.
     */
    void enableHwCompositing(LOutput *output, bool enabled) noexcept;

    /**
     * @brief Checks if hardware compositing is enabled for the specified output.
     *
     * @note This method always returns `false` if hasHardwareSupport() for the given output returns `false`.
     *
     * @param output The output to check.
     * @return `true` if hardware compositing is enabled for the given output, `false` otherwise.
     */
    bool hwCompositingEnabled(LOutput *output) const noexcept;

    /**
     * @brief Damaged region in compositor-global coordinates.
     *
     * Provides the region that needs to be repainted when hardware compositing isn't supported or is disabled,
     * allowing Louvre to properly render it after an LOutput::paintGL() event.
     *
     * @return The region that needs to be repainted.
     */
    const LRegion &damage(LOutput *output) const noexcept;

    /**
     * @brief Gets the current cursor output.
     *
     * Returns the output where the cursor is currently positioned.
     *
     * @note This method always returns a valid output unless there are none initialized.
     */
    LOutput *output() const noexcept;

    /**
     * @brief Vector of intersected outputs.
     *
     * Returns a vector of initialized outputs that intersect with the cursor's rect() property.
     */
    const std::vector<LOutput*> &intersectedOutputs() const noexcept;

    /**
     * @brief Repaint intersected outputs.
     *
     * Invokes LOutput::repaint() for each output in the vector of intersected outputs.
     *
     * @param nonHardwareOnly If `true`, only repaints outputs that do not support hardware compositing.
     */
    void repaintOutputs(bool nonHardwareOnly = true) noexcept;

    LPRIVATE_IMP_UNIQUE(LCursor)
    friend class Louvre::LCompositor;
    friend class Louvre::LOutput;
    LCursor() noexcept;
    ~LCursor();

    LPointF m_pos;
    mutable LWeak<LOutput> m_output;
};

#endif // LCURSOR_H
