#ifndef LPAINTER_H
#define LPAINTER_H

#include <LObject.h>
#include <LPoint.h>

/*!
 * @brief Renderer utilities.
 *
 * The LPainter class offers basic functions for 2D rendering without the need to use OpenGL directly. It can draw texture rects or solid colors, clear the screen and set the viewport.\n
 * Its goal is to abstract the rendering API, allowing for portable compositors, independent of the renderer used.\n
 * Currently, the library only offers the OpenGL ES 2.0 renderer, but in the future others such as Vulkan, Pixman, etc. could be incorporated.\n
 * Each LOutput has its own LPainter, which can be accessed from LOutput::painter().\n
 * @warning It is not mandatory to use LPainter's functions for rendering, the library allows the developer to use OpenGL functions directly if desired.
 */
class Louvre::LPainter : LObject
{
public:
    LPainter(const LPainter&) = delete;
    LPainter& operator= (const LPainter&) = delete;

    void bindFramebuffer(LFramebuffer *framebuffer);

    /*!
     * @brief Draws a texture.
     *
     * Draws a rect or sub-rect of a texture on screen.
     *
     * @param texture Texture to draw.
     * @param srcB The portion of the texture to draw (specified in buffer coordinates).
     * @param dstG The portion of the screen where the texture will be drawn (specified in compositor coordinates).
     * @param alpha Value of the alpha component (range [0.0, 1.0]).
     */
    void drawTextureC(const LTexture *texture, const LRect &srcB, const LRect &dstG,
                      Float32 srcScale = 0.f, Float32 alpha = 1.f);

    void drawTextureC(const LTexture *texture,
                      Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                      Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                      Float32 srcScale = 0.f, Float32 alpha = 1.0f);

    /*!
     * @brief Draws a solid color.
     *
     * Draws a rectangle filled with a solid color on the screen.
     *
     * @param dst The portion of the screen where the rectangle will be drawn (specified in compositor coordinates).
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void drawColorC(const LRect &dst,
                    Float32 r, Float32 g, Float32 b, Float32 a);

    void drawColorC(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                    Float32 r, Float32 g, Float32 b, Float32 a);

    /*!
     * @brief Sets the viewport.
     *
     * @param rect Viewport rect specified in compositor coordinates.
     */
    void setViewportC(const LRect &rect);
    void setViewportC(Int32 x, Int32 y, Int32 w, Int32 h);

    /*!
     * @brief Sets the clear color.
     *
     * Sets the clear color used when calling clearScreen().
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a);

    /*!
     * @brief Clears the screen.
     *
     * Clears the screen using the color defined by setClearColor().
     */
    void clearScreen();

    /*!
     * @brief Uses LPainter's program.
     *
     * If you are using your own OpenGL programs, remember to call this method before using LPainter's functions.
     */
    void bindProgram();

    LPRIVATE_IMP(LPainter)

    friend class LCompositor;
    friend class LOutput;
    LPainter();
    ~LPainter();
};

#endif // LPAINTER_H
