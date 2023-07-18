#ifndef LCURSOR_H
#define LCURSOR_H

#include <LObject.h>
#include <LPoint.h>
#include <LTexture.h>
#include <LRect.h>

/*!
 * @brief Utility class for rendering cursors.
 *
 * The LCursor class is designed to make cursor rendering easier and take advantage of compositing properties of 
 * certain graphics backends to improve performance.\n
 *
 * @subsection init Initialization
 *
 * There is a single instance of LCursor accessible with LCompositor::cursor() which is created during initialization
 * of the first output (LOutput) added to the compositor with LCompositor::addOutput() and notified with LCompositor::cursorInitialized().\n
 * The cursor must always be part of at least one output. To select the current output the setOutput() method must be used.\n
 *
 * @warning The library automatically sets the current output of the cursor based on its position, so it is not necessary to use the setOutput() method directly unless you are rendering the cursor in a coordinate system other than compositor space.
 *
 * @subsection hw_composition Hardware Composition
 *
 * Some graphics backends, such as DRM, allow for hardware cursor compositing, which can improve performance by reducing the need to repaint 
 * an output every time the cursor changes position.\n
 * To check if a backend supports hardware cursor compositing, use the hasHardwareSupport() method.\n
 * If hardware compositing is not supported, the cursor must be rendered using OpenGL. 
 * In this case, the cursor's position and size in compositor coordinates can be accessed using the rectC() method, and its texture with the texture() method.
*/
class Louvre::LCursor : public LObject
{
public:

    LCursor(const LCursor&) = delete;
    LCursor& operator= (const LCursor&) = delete;

    /*!
     * @brief Sets the library's default cursor.
     *
     * Sets the texture and hotspot of the default cursor shipped with the library.
     *
     * <center><IMG SRC="https://lh3.googleusercontent.com/MSUUg3LSS6lYtpyLnKzbECf9eeZeFscmnLGJLRCdADwcjjcVd4xT07AMvQoHUTGptJFzY4tZrQ3IdLKyEbM_O0WyWYk8Pvc-Jf8xZHXoFUkFo2RRYTP8zN_LeOhsvIc6SlsO83TJUw=w2400"></center>
     *
     * The default cursor is the one shown in the image, with a size of 64x64 pixels and hotspot at (8,8).
     */
    void useDefault();

    /*!
     * @brief Sets the cursor texture.
     *
     * Assigns the texture and hotspot of the cursor. The texture size does not necessarily define the cursor size, setSizeS() must be used to assign the cursor size.\n 
     *
     * @param texture Texture to assign.
     * @param hotspot Cursor hotspot in buffer coordinates.
     */
    void setTextureB(const LTexture *texture, const LPointF &hotspot);

    /*!
     * @brief Current texture.
     *
     * Pointer to the current cursor texture.
     */
    LTexture *texture() const;

    /*!
     * @brief Assigns the current output.
     *
     * The library automatically assigns the current output based on the cursor position.\n
     * The cursor must always have an assigned output so that the library can communicate correctly with the graphic backend in
     * cases where hardware composition is available.\n
     */
    void setOutput(LOutput *output);

    /*!
     * @brief Moves the cursor.
     *
     * Modifies the position of the cursor by a delta (dx, dy).
     *
     * @param dx Delta x in compositor coordinates.
     * @param dy Delta y in compositor coordinates.
     */
    void moveC(float dx, float dy);

    /*!
     * @brief Assigns the cursor position.
     *
     * @param pos Cursor position in compositor coordinates.
     */
    void setPosC(const LPointF &pos);

    /*!
     * @brief Cursor position.
     *
     * Cursor position in compositor coordinates.
     */
    const LPointF &posC() const;

    /*!
     * @brief Cursor rect on screen.
     *
     * Returns the cursor rect on screen in compositor coordinates LRect(posC - hotspotG, sizeC). It may be used to render the cursor with LPainter when hardware compositing is not available.
     */
    const LRect &rectC() const;

    /*!
     * @brief Assigns the cursor hotspot.
     *
     * The cursor hotspot are coordinates relative to the origin of its buffer (upper left corner) used to position it correctly on the screen.\n
     * For example, if the texture was a cross with size (64,64), the hotspot would be in the center (32.32). Therefore, if the position of the cursor
     * is (x,y), its final location on screen would be (x - 32, y - 32).
     *
     * @param Hotspot hotspot in buffer coordinates.
     */
    void setHotspotB(const LPointF &hotspot);

    /*!
     * @brief Cursor hotspot.
     *
     * Current hotspot in buffer coordinates.
     */
    const LPointF &hotspotB()  const;

    /*!
     * @brief Assigns the cursor size.
     *
     * Assigns cursor size using surface coordinates. The texture and hotspot are automatically scaled, the latter maintaining its proportion to the buffer size.\n
     * Being surface coordinates, the cursor automatically adjusts to the scale of the current output.
     *
     * @param size Cursor size in surface coordinates.
     */
    void setSizeS(const LSizeF &size);

    /*!
     * @brief Change cursor visibility.
     *
     * Shows or hides the cursor. If the cursor does not support hardware composition it is the developer's responsibility to avoid rendering it when it is hidden.\n
     * You can use visible() to know if it is visible.
     * @param state True makes the cursor visible and False hides it.
     */
    void setVisible(bool state);

    /*!
     * @brief Indicates whether the cursor is visible.
     *
     * @returns True if visible and False if hidden.
     */
    bool visible() const;

    /*!
     * @brief Repaints the outputs.
     *
     * Invokes the LOutput::repaint() method on all outputs where the cursor is currently visible.
     */
    void repaintOutputs();

    /*!
     * @brief Indicates whether the backend supports hardware compositing.
     *
     * @returns True if the backend supports it and False otherwise.
     */
    bool hasHardwareSupport(const LOutput *output) const;

    /*!
     * @brief Current cursor output.
     */
    LOutput *output() const;

    /*!
     * @brief List of outputs intersected by the cursor.
     */
    const std::list<LOutput*>&intersectedOutputs() const;

    LPRIVATE_IMP(LCursor)

    friend class Louvre::LCompositor;
    friend class Louvre::LOutput;

    /*!
     * @brief Constructor of the LCursor class.
     * @param output The output on which the cursor is initialized.
     */
    LCursor();

    /*!
     * @brief Desctructor of the LCursor class.
     */
    ~LCursor();
};

#endif // LCURSOR_H
