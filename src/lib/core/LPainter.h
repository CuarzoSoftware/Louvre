#ifndef LPAINTER_H
#define LPAINTER_H

#include <LObject.h>
#include <LPoint.h>

/**
 * @brief Renderer painting functions
 *
 * The LPainter class offers basic functions for 2D rendering without the need to use OpenGL functions directly. It can draw texture rects or solid colors, clear the screen and set the viewport.\n
 * Its goal is to abstract the rendering API, allowing for portable compositors, independent of the renderer used.\n
 * Currently, the library only offers the OpenGL ES 2.0 renderer, but in the future others such as Vulkan, Pixman, etc. could be incorporated.\n
 * Each LOutput has its own LPainter, which can be accessed from LOutput::painter().\n
 *
 * @note It is not mandatory to use LPainter functions for rendering, you can use OpenGL functions and your own shaders if desired.
 */
class Louvre::LPainter : LObject
{
public:
    /// @cond OMIT
    LPainter(const LPainter&) = delete;
    LPainter& operator= (const LPainter&) = delete;
    /// @endcond

    /**
     * @brief Binds the specified framebuffer for rendering.
     *
     * This function binds the provided framebuffer for rendering, allowing subsequent rendering operations to be performed on it.
     *
     * @param framebuffer The framebuffer to be bound.
     */
    void bindFramebuffer(LFramebuffer *framebuffer);

    /**
     * @brief Retrieves the currently bound framebuffer.
     *
     * This function returns a pointer to the currently bound framebuffer for rendering.
     *
     * @return A pointer to the currently bound framebuffer.
     */
    LFramebuffer *boundFramebuffer() const;

    /**
     * @brief Draws a texture.
     *
     * Draws a texture or sub-rect of a texture on the screen.
     *
     * @note Using 1 as the `srcScale` allows you to define the `src` rect in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param src The portion of the texture to draw (specified in surface coordinates).
     * @param dst The portion of the screen/framebuffer where the texture will be drawn (specified in surface coordinates).
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawTexture(const LTexture *texture, const LRect &src, const LRect &dst,
                     Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draws a texture.
     *
     * Draws a portion of a texture onto the screen at a specific position and size.
     *
     * @note Using 1 as the `srcScale` allows you to define the source rect in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param srcX X-coordinate of the source rectangle.
     * @param srcY Y-coordinate of the source rectangle.
     * @param srcW Width of the source rectangle.
     * @param srcH Height of the source rectangle.
     * @param dstX X-coordinate of the destination rectangle.
     * @param dstY Y-coordinate of the destination rectangle.
     * @param dstW Width of the destination rectangle.
     * @param dstH Height of the destination rectangle.
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawTexture(const LTexture *texture,
                     Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                     Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                     Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draws a solid color using a texture's alpha channel, with support for HiDPI scaling.
     *
     * This function draws a rectangle or sub-rectangle of a texture on the screen, while maintaining its alpha channel,
     * and replaces the original color with a solid color specified by the user.
     *
     * @note Using 1 as the `srcScale` allows you to define the `src` rect in buffer coordinates.
     *
     * @param texture A pointer to the texture to be drawn.
     * @param color The solid color (LRGBF format) that will replace the original texture color.
     * @param src The source rectangle within the texture that defines the region to be drawn.
     * @param dst The destination rectangle on the screen where the texture will be drawn.
     * @param srcScale Scale of the texture buffer.
     * @param alpha The alpha (transparency) value of the texture (default is 1.0f, fully opaque).
     */
    void drawColorTexture(const LTexture *texture, const LRGBF &color, const LRect &src, const LRect &dst,
                          Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draws a solid color using a texture's alpha channel, with support for HiDPI scaling.
     *
     * This function draws a rectangle or sub-rectangle of a texture on the screen, while maintaining its alpha channel,
     * and replaces the original color with a solid color specified by the user. It also provides support for HiDPI scaling,
     * allowing the user to control the scaling of the source rectangle when rendering on HiDPI (High-DPI) buffers.
     *
     * @note Using 1 as the `srcScale` allows you to define the source rect in buffer coordinates.
     *
     * @param texture A pointer to the texture to be drawn.
     * @param r The red component of the solid color (0.0 to 1.0).
     * @param g The green component of the solid color (0.0 to 1.0).
     * @param b The blue component of the solid color (0.0 to 1.0).
     * @param srcX The x-coordinate of the top-left corner of the source rectangle in the texture.
     * @param srcY The y-coordinate of the top-left corner of the source rectangle in the texture.
     * @param srcW The width of the source rectangle in the texture.
     * @param srcH The height of the source rectangle in the texture.
     * @param dstX The x-coordinate of the top-left corner of the destination rectangle on the screen.
     * @param dstY The y-coordinate of the top-left corner of the destination rectangle on the screen.
     * @param dstW The width of the destination rectangle on the screen.
     * @param dstH The height of the destination rectangle on the screen.
     * @param srcScale The scaling factor applied to the source rectangle, useful for HiDPI buffers (default is 1.0f, no scaling).
     * @param alpha The alpha (transparency) value of the texture (default is 1.0f, fully opaque).
     */
    void drawColorTexture(const LTexture *texture, Float32 r, Float32 g, Float32 b,
                          Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                          Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                          Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draws a solid color.
     *
     * Draws a colored rectangle on the screen.
     *
     * @param dst The portion of the screen where the rectangle will be drawn (specified in surface coordinates).
     * @param r Red component value (range [0.0, 1.0]).
     * @param g Green component value (range [0.0, 1.0]).
     * @param b Blue component value (range [0.0, 1.0]).
     * @param a Alpha component value (range [0.0, 1.0]).
     */
    void drawColor(const LRect &dst, Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Draws a solid color.
     *
     * Draws a colored rectangle on the screen.
     *
     * @param dstX X-coordinate of the destination rectangle.
     * @param dstY Y-coordinate of the destination rectangle.
     * @param dstW Width of the destination rectangle.
     * @param dstH Height of the destination rectangle.
     * @param r Red component value (range [0.0, 1.0]).
     * @param g Green component value (range [0.0, 1.0]).
     * @param b Blue component value (range [0.0, 1.0]).
     * @param a Alpha component value (range [0.0, 1.0]).
     */
    void drawColor(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                   Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Sets the viewport.
     *
     * @note This function should only be used if you are working with your own shaders/programs.
     *
     * @param rect Viewport rectangle specified in compositor coordinates.
     */
    void setViewport(const LRect &rect);

    /**
     * @brief Sets the viewport.
     *
     * @note This function should only be used if you are working with your own shaders/programs.
     *
     * @param x X-coordinate of the viewport rectangle.
     * @param y Y-coordinate of the viewport rectangle.
     * @param w Width of the viewport rectangle.
     * @param h Height of the viewport rectangle.
     */
    void setViewport(Int32 x, Int32 y, Int32 w, Int32 h);

    /**
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

    /**
     * @brief Clears the screen.
     *
     * Clears the screen using the color defined by setClearColor().
     */
    void clearScreen();

    /**
     * @brief Uses LPainter's program.
     *
     * If you are using your own OpenGL programs, remember to call this method before using LPainter functions.
     */
    void bindProgram();

    LPRIVATE_IMP(LPainter)

    /// @cond OMIT
    friend class LCompositor;
    friend class LOutput;
    LPainter();
    ~LPainter();
    /// @endcond
};

#endif // LPAINTER_H
