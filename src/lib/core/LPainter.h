#ifndef LPAINTER_H
#define LPAINTER_H

#include <LObject.h>
#include <LPoint.h>
#include <LFramebuffer.h>
#include <LRect.h>
#include <LColor.h>
#include <memory>

#if LOUVRE_USE_SKIA == 1
#include <include/gpu/GrDirectContext.h>
#endif

/**
 * @brief Basic 2D rendering utility
 *
 * The LPainter class offers essential methods for 2D rendering without the need to use OpenGL functions directly.\n
 * It can draw solid colors and textures with custom clipping, scaling, blending effects, and transforms.
 *
 * Each LOutput has its own LPainter, accessible via LOutput::painter().
 *
 * @note While LPainter provides convenience, it's not mandatory. You have the flexibility to use OpenGL functions and your custom shaders if preferred,
 *       or leverage the efficiency of rendering provided by the LScene and LView classes.
 *
 * ## Coordinate System
 *
 * When specifying the destination rectangle for your painting operations, use compositor-global coordinates, which operate with surface units.\n
 * LPainter automatically scales and transforms the content for you, considering the scale factor/transform of the texture you are drawing and the scale factor/transform
 * of the destination framebuffer.
 *
 * The coordinate space in which the content is rendered aligns with that used to arrange outputs.\n
 * For example, to paint something in the upper-left corner of an LOutput, consider the LOutput::pos().
 *
 * @note When rendering into an LRenderBuffer, also consider its position, similar to how you handle outputs.
 */
class Louvre::LPainter final : LObject
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
        LTexture *texture;

        /**
         * @brief Position of the texture (destination rect) in compositor-global coordinates.
         */
        LPoint pos;

        /**
         * @brief Subrect of the texture to be mapped in surface units.
         *
         * Coordinates should be specified in the space generated after applying the scale factor and transformation to the texture buffer.
         */
        LRectF srcRect;

        /**
         * @brief Destination size of the source rect in surface units.
         */
        LSize dstSize;

        /**
         * @brief Transform already applied to the texture.
         *
         * For example, if the texture is rotated 90 degrees counterclockwise and you want to display it in a normal orientation,
         * use LTransform::Rotated90 and LPainter will apply the inverse transform (LTransform::Rotated270).\n
         * If you don't want to apply any transform use LTransform::Normal.
         */
        LTransform srcTransform { LTransform::Normal };

        /**
         * @brief Scale factor of the texture.
         */
        Float32 srcScale { 1.f };
    };

    /**
     * @brief Switches to texture mode.
     *
     * This method maps a texture to the compositor-global space, enabling subsequent drawing of specific sub-rectangles using drawBox(), drawRect() or drawRegion().
     *
     * @param params Parameters required to map the texture.
     */
    void bindTextureMode(const TextureParams &params) noexcept;

    /**
     * @brief Switches to color mode.
     *
     * In color mode, drawBox(), drawRect() and drawRegion() can be used to draw rectagles of the specified color.
     *
     * The color is set using setColor() and setAlpha().
     */
    void bindColorMode() noexcept;

    /**
     * @brief Draws a texture or color box on the screen based on the current rendering mode.
     *
     * @param box The box to be drawn in compositor-global coordinates.
     */
    void drawBox(const LBox &box) noexcept;

    /**
     * @brief Draws a texture or color rect on the screen based on the current rendering mode.
     *
     * @param rect The rect to be drawn in compositor-global coordinates.
     */
    void drawRect(const LRect &rect) noexcept;

    /**
     * @brief Draws a texture or color region on the screen based on the current rendering mode.
     *
     * @param region The region to be drawn in compositor-global coordinates.
     */
    void drawRegion(const LRegion &region) noexcept;

    /**
     * @brief Enables or disables a custom texture color.
     *
     * When enabled, the bound texture RGB values are replaced by the color set with setColor().
     */
    void enableCustomTextureColor(bool enabled) noexcept;

    /**
     * @brief Checks if custom texture color is enabled.
     *
     * @see enableCustomTextureColor().
     *
     * @return `true` if custom texture color is enabled, otherwise `false`.
     */
    bool customTextureColorEnabled() const noexcept;

    /**
     * @brief Checks if automatic blending function selection is enabled.
     *
     * @see enable enableAutoBlendFunc().
     */
    bool autoBlendFuncEnabled() const noexcept;

    /**
     * @brief Toggles automatic blending function selection.
     *
     * When enabled, LPainter will automatically select the appropriate OpenGL blend function mode
     * based on whether the bound texture uses premultiplied alpha or not (see LTexture::premultipliedAlpha()).
     *
     * When disabled, LPainter will use the blend function set with setBlendFunc().
     *
     * Enabled by default.
     */
    void enableAutoBlendFunc(bool enabled) const noexcept;

    /**
     * @brief Sets a custom blend function.
     *
     * Setting a custom blend function can be useful for masking or other sophisticated blending effects.
     *
     * To make LPainter use this blend function, the auto blend function must be disabled, see enableAutoBlendFunc().
     */
    void setBlendFunc(const LBlendFunc &blendFunc) const noexcept;

    /**
     * @brief Sets the alpha value.
     *
     * - In texture mode, the texture alpha value is multiplied by this value.
     * - In color mode, this value becomes the alpha value of the color.
     *
     * @param alpha The alpha value to be set.
     */
    void setAlpha(Float32 alpha) noexcept;

    /**
     * @brief Sets the color.
     *
     * - In texture mode and if customTextureColorEnabled() is enabled, this value replaces the texture RGB values while keeping the alpha intact.
     * - If color mode, this is the color to be drawn. See setAlpha().
     *
     * @param color The color to be set.
     */
    void setColor(const LRGBF &color) noexcept;

    /**
     * @brief Sets the color factor.
     *
     * This method multiplies each component of the source color by the specified factor.
     *
     * @note Setting all components to 1.0 disables the effect.
     *
     * @param r The value of the red component (range: [0.0, 1.0]).
     * @param g The value of the green component (range: [0.0, 1.0]).
     * @param b The value of the blue component (range: [0.0, 1.0]).
     * @param a The value of the alpha component (range: [0.0, 1.0]).
     */
    void setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept;

    /**
     * @brief Sets the color factor.
     *
     * This method multiplies each component of the source color by the specified factor.
     *
     * @note Setting all components to 1.0 disables the effect.
     *
     * @param factor The color factor to be set.
     */
    void setColorFactor(const LRGBAF &factor) noexcept;

    /**
     * @brief Sets the clear color.
     *
     * This method sets the clear color used when calling clearScreen().
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept;

    /**
     * @brief Set the clear color.
     *
     * This method sets the clear color used when calling clearScreen().
     */
    void setClearColor(const LRGBAF &color) noexcept;

    /**
     * @brief Clear the framebuffer.
     *
     * This method clears the bound framebuffer using the color set with setClearColor().
     */
    void clearScreen() noexcept;

    /**
     * @brief Sets the viewport.
     *
     * @note Do not use this method unless you are working with your own shaders/programs.
     */
    void setViewport(const LRect &rect) noexcept;

    /**
     * @brief Sets the viewport.
     *
     * @note Do not use this method unless you are working with your own shaders/programs.
     */
    void setViewport(Int32 x, Int32 y, Int32 w, Int32 h) noexcept;

    /**
     * @brief Binds the specified framebuffer for rendering.
     *
     * This method binds the provided framebuffer for rendering, allowing subsequent rendering operations to be performed on it.
     *
     * @note Output framebuffers are automatically bound prior a LOutput::paintGL() event.
     *
     * @param framebuffer The framebuffer to be bound.
     */
    void bindFramebuffer(LFramebuffer *framebuffer) noexcept;

    /**
     * @brief Retrieves the currently bound framebuffer.
     *
     * This method returns a pointer to the currently bound framebuffer for rendering.
     *
     * @return A pointer to the currently bound framebuffer.
     */
    LFramebuffer *boundFramebuffer() const noexcept;

    /**
     * @brief Bind the internal LPainter program.
     *
     * @note This method should be used if you are working with your own OpenGL programs and want to use the LPainter methods again.
     */
    void bindProgram() noexcept;

#if LOUVRE_USE_SKIA == 1
    // TODO: Add doc
    GrDirectContext *skContext() const noexcept;
#endif

    LPRIVATE_IMP_UNIQUE(LPainter)

    friend class LCompositor;
    friend class LOutput;
    LPainter() noexcept;
    ~LPainter() noexcept;
};

#endif // LPAINTER_H
