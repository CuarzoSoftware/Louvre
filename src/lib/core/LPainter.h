#ifndef LPAINTER_H
#define LPAINTER_H

#include <LObject.h>
#include <LPoint.h>
#include <LFramebuffer.h>
#include <LRect.h>

/**
 * @brief Utility for Painting Operations
 *
 * The LPainter class offers basic methods for 2D rendering without the need to use OpenGL functions directly.
 * It can draw texture rects or solid colors, clear the screen and set the viewport.\n
 * Its goal is to abstract the rendering API, allowing for portable compositors, independent of the renderer used.\n
 * Currently, the library only offers the OpenGL ES 2.0 renderer, but in the future others such as Vulkan, Pixman, etc. could be incorporated.\n
 * Each LOutput has its own LPainter, which can be accessed from LOutput::painter().\n
 *
 * @note You are not obligated to use LPainter methods for rendering. You have the flexibility to use OpenGL functions and your
 *       custom shaders if preferred, or leverage the LScene and LView classes for efficient rendering.
 *
 * ## Coordinate System
 *
 * When specifying the destination rect for your painting operations, you must use surface coordinates.
 * LPainter automatically scales the content for you, taking into account the scale factor of the texture you are
 * drawing and the scale factor of the destination framebuffer.
 *
 * The coordinate space in which the content is rendered is the same as that used to arrange the outputs.
 * For instance, if you want to paint something in the upper-left corner of an LOutput, you must consider
 * the LOutput's position.
 *
 * @note When rendering into an LRenderBuffer, you should also consider its position, similar to how you do with outputs.
 */
class Louvre::LPainter : LObject
{
public:

    /**
     * @brief Parameters required for bindTextureMode().
     *
     * This struct provides all the necessary parameters to map a texture into the current destination framebuffer.
     */
    struct TextureParams
    {
        /**
         * @brief Texture to be drawn.
         */
        const LTexture *texture;

        /**
         * @brief Position of the texture (destination rect) in global compositor coordinates.
         */
        LPoint pos;

        /**
         * @brief Subrect of the texture to be mapped in surface coordinates with fractional values.
         *
         * Coordinates should be specified in the space generated after applying the scale factor and transformation to the texture buffer.
         */
        LRectF srcRect;

        /**
         * @brief Destination size of the source rect in surface coordinates.
         */
        LSize dstSize;

        /**
         * @brief Transform applied to the texture.
         *
         * For example, if the texture is rotated 90 degrees counterclockwise, use LFramebuffer::Rotated90.
         * LPainter will apply the inverse transform (LFramebuffer::Rotated270).
         */
        LFramebuffer::Transform srcTransform = LFramebuffer::Normal;

        /**
         * @brief Scale factor of the texture.
         */
        Float32 srcScale = 1.f;
    };

    /**
     * @brief Switches to texture mode.
     *
     * This method maps a texture to the global compositor space, enabling subsequent drawing of specific parts using drawBox(), drawRect() or drawRegion().
     *
     * @param params Parameters required to map the texture.
     */
    void bindTextureMode(const TextureParams &params);

    /**
     * @brief Switches to color mode.
     *
     * In color mode, drawBox() or drawRect() can be called to draw the specified color.
     * The color is set using setColor() and setAlpha().
     */
    void bindColorMode();

    /**
     * @brief Draws a texture or color box on the screen based on the current rendering mode.
     *
     * @param box The box to be drawn in global compositor coordinates.
     */
    void drawBox(const LBox &box);

    /**
     * @brief Draws a texture or color rect on the screen based on the current rendering mode.
     *
     * @param rect The rect to be drawn in global compositor coordinates.
     */
    void drawRect(const LRect &rect);

    /**
     * @brief Draws a texture or color region on the screen based on the current rendering mode.
     *
     * @param region The region to be drawn in global compositor coordinates.
     */
    void drawRegion(const LRegion &region);

    /**
     * @brief Enables or disables custom texture color.
     *
     * When enabled, the texture RGB values are replaced by the color set with setColor().
     */
    void enableCustomTextureColor(bool enabled);

    /**
     * @brief Checks if custom texture color is enabled.
     *
     * @return `true` if custom texture color is enabled, otherwise `false`.
     */
    bool customTextureColorEnabled() const;

    /**
     * @brief Sets the alpha value.
     *
     * In texture mode, the texture alpha value is multiplied by this value.
     * In color mode, this value becomes the alpha value of the color.
     *
     * @param alpha The alpha value to be set.
     */
    void setAlpha(Float32 alpha);

    /**
     * @brief Sets the color.
     *
     * If the current mode is texture mode and customTextureColorEnabled() is enabled, this value replaces the texture RGB values while keeping the alpha intact.
     * If the current mode is color mode, this is the color to be drawn.
     *
     * @param color The color to be set.
     */
    void setColor(const LRGBF &color);

    /**
     * @brief Sets the color factor.
     *
     * This factor multiplies each component of the destination color. Setting all components to 1.0f disables it.
     *
     * @param color The color factor to be set.
     */
    void setColorFactor(const LRGBAF &color);

    /// @cond OMIT
    LPainter(const LPainter&) = delete;
    LPainter& operator= (const LPainter&) = delete;
    /// @endcond

    /**
     * @brief Bind the specified framebuffer for rendering.
     *
     * This method binds the provided framebuffer for rendering, allowing subsequent rendering operations to be performed on it.
     *
     * @note Output framebuffers are automatically bound prior a LOutput::paintGL() event.
     *
     * @param framebuffer The framebuffer to be bound.
     */
    void bindFramebuffer(LFramebuffer *framebuffer);

    /**
     * @brief Retrieve the currently bound framebuffer.
     *
     * This method returns a pointer to the currently bound framebuffer for rendering.
     *
     * @return A pointer to the currently bound framebuffer.
     */
    LFramebuffer *boundFramebuffer() const;

    /**
     * @brief Draw a texture.
     *
     * This method allows you to draw a texture or sub-rect of a texture on the bound frambuffer.
     *
     * @note Using 1 as the `srcScale` means that the `src` rect is in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param src The portion of the texture to draw specified in surface coordinates.
     * @param dst The destination rect where the texture will be drawn specified in surface coordinates.
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawTexture(const LTexture *texture, const LRect &src, const LRect &dst,
                     Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draw a texture.
     *
     * This method allows you to draw a texture or sub-rect of a texture on the bound frambuffer.
     *
     * @note Using 1 as the `srcScale` means that the `src` rect is in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param srcX X-coordinate of the source rect.
     * @param srcY Y-coordinate of the source rect.
     * @param srcW Width of the source rect.
     * @param srcH Height of the source rect.
     * @param dstX X-coordinate of the destination rect.
     * @param dstY Y-coordinate of the destination rect.
     * @param dstW Width of the destination rect.
     * @param dstH Height of the destination rect.
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawTexture(const LTexture *texture,
                     Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                     Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                     Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draw a texture with a custom solid color.
     *
     * This method draws a rect or sub-rect of a texture, replacing its original color while maintaining its alpha channel.
     *
     * @note Using 1 as the `srcScale` means that the `src` rect is in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param color The solid color that will replace the original texture color.
     * @param src The portion of the texture to draw specified in surface coordinates.
     * @param dst The destination rect where the texture will be drawn specified in surface coordinates.
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawColorTexture(const LTexture *texture, const LRGBF &color, const LRect &src, const LRect &dst,
                          Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draw a texture with a custom solid color.
     *
     * This method draws a rect or sub-rect of a texture, replacing its original color while maintaining its alpha channel.
     *
     * @note Using 1 as the `srcScale` means that the `src` rect is in buffer coordinates.
     *
     * @param texture Texture to draw.
     * @param r The red component of the solid color (0.0 to 1.0).
     * @param g The green component of the solid color (0.0 to 1.0).
     * @param b The blue component of the solid color (0.0 to 1.0).
     * @param srcX X-coordinate of the source rect.
     * @param srcY Y-coordinate of the source rect.
     * @param srcW Width of the source rect.
     * @param srcH Height of the source rect.
     * @param dstX X-coordinate of the destination rect.
     * @param dstY Y-coordinate of the destination rect.
     * @param dstW Width of the destination rect.
     * @param dstH Height of the destination rect.
     * @param srcScale Scaling factor for the source texture (default is 1.0).
     * @param alpha Alpha value for blending the texture (range [0.0, 1.0]).
     */
    void drawColorTexture(const LTexture *texture, Float32 r, Float32 g, Float32 b,
                          Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                          Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                          Float32 srcScale = 1.0f, Float32 alpha = 1.0f);

    /**
     * @brief Draw a solid color rect.
     *
     * This method draws a solid colored rect.
     *
     * @param dst The destination rect where the color will be drawn specified in surface coordinates.
     * @param r Red component value (range [0.0, 1.0]).
     * @param g Green component value (range [0.0, 1.0]).
     * @param b Blue component value (range [0.0, 1.0]).
     * @param a Alpha component value (range [0.0, 1.0]).
     */
    void drawColor(const LRect &dst, Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Draw a solid color rect.
     *
     * This method draws a solid colored rect.
     *
     * @param dstX X-coordinate of the destination rect.
     * @param dstY Y-coordinate of the destination rect.
     * @param dstW Width of the destination rect.
     * @param dstH Height of the destination rect.
     * @param r Red component value (range [0.0, 1.0]).
     * @param g Green component value (range [0.0, 1.0]).
     * @param b Blue component value (range [0.0, 1.0]).
     * @param a Alpha component value (range [0.0, 1.0]).
     */
    void drawColor(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                   Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Set the viewport.
     *
     * @note This method should be used if you are working with your own shaders/programs.
     */
    void setViewport(const LRect &rect);

    /**
     * @brief Sets the viewport.
     *
     * @note This method should be used if you are working with your own shaders/programs.
     */
    void setViewport(Int32 x, Int32 y, Int32 w, Int32 h);

    /**
     * @brief Set the clear color.
     *
     * This method sets the clear color used when calling clearScreen().
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Set the clear color.
     *
     * This method sets the clear color used when calling clearScreen().
     */
    void setClearColor(const LRGBAF &color);

    /**
     * @brief Set the color factor.
     *
     * This method allows you to set a color factor that influences the resulting color of every painting operation.
     * By default, the color factor is (1.0, 1.0, 1.0, 1.0), which has no effect on the colors.
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Clear the framebuffer.
     *
     * This method clears the bound framebuffer using the color set with setClearColor().
     */
    void clearScreen();

    /**
     * @brief Bind the internal LPainter program.
     *
     * @note This method should be used if you are working with your own OpenGL programs and want to use the LPainter methods again.
     */
    void bindProgram();

    LPRIVATE_IMP_UNIQUE(LPainter)

    /// @cond OMIT
    friend class LCompositor;
    friend class LOutput;
    LPainter();
    ~LPainter();
    /// @endcond
};

#endif // LPAINTER_H
