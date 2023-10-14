#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>

/**
 * @brief View for displaying textures
 *
 * The LTextureView class enables you to use an LTexture as a view within a scene.\n
 * You can set the view's texture using setTexture(), and passing `nullptr` unsets the texture, effectively unmapping the view.\n
 * Multiple texture views can share the same LTexture, and if the texture is destroyed, their texture() property is automatically set to `nullptr`.
 *
 * Texture views can also have a custom destination size, which may differ from their buffer size. In such cases, regions for damage, input, translucent,
 * and opaque must be defined based on the destination size.\n
 * To enable a custom destination size, utilize the enableDstSize() and setDstSize() methods.\n
 *
 * @note Using a custom destination size is recommended instead of relying on the scalingVector() option, as it allows for continued damage tracking
 *       within the scene.
 *
 * When destination size is disabled, the view size is by default equal to the texture size divided by its buffer scale if no other transformations
 * are applied.
 *
 * For additional methods and properties available, please refer to the documentation of the `LView` class.
 */
class Louvre::LTextureView : public LView
{
public:
    /// @cond OMIT
    LTextureView(const LTextureView&) = delete;
    LTextureView& operator= (const LTextureView&) = delete;
    /// @endcond

    /**
     * @brief Construct an LTextureView with an optional LTexture and parent LView.
     *
     * @param texture The LTexture to be used as the view's texture. Default is nullptr.
     * @param parent The parent LView of the LTextureView. Default is nullptr.
     */
    LTextureView(LTexture *texture = nullptr, LView *parent = nullptr);

    /**
     * @brief Destructor for the LTextureView.
     */
    ~LTextureView();

    /**
     * @brief Set the position of the LTextureView.
     *
     * @param x The x-coordinate of the position.
     * @param y The y-coordinate of the position.
     */
    virtual void setPos(Int32 x, Int32 y);

    /**
     * @brief Set the position of the LTextureView using an LPoint object.
     *
     * @param pos The position as an LPoint object.
     */
    void setPos(const LPoint &pos);

    /**
     * @brief Set the input region of the LTextureView.
     *
     * @param region The input region as an LRegion object.
     */
    virtual void setInputRegion(const LRegion *region);

    /**
     * @brief Set the translucent region of the LTextureView.
     *
     * Passing `nullptr` as the region means that the entire view is considered translucent,
     * which is the default value.
     * Passing an empty LRegion (not `nullptr`), on the other hand, means the entire view is opaque.
     *
     * @param region The translucent region as an LRegion object.
     */
    virtual void setTranslucentRegion(const LRegion *region);

    /**
     * @brief Set the buffer scale of the LTextureView.
     *
     * @param scale The buffer scale as an Int32 value.
     */
    virtual void setBufferScale(Int32 scale);

    /**
     * @brief Set the LTexture for the LTextureView.
     *
     * @note If the current texture is destroyed, this property is automatically set to `nullptr`.
     *
     * @param texture The LTexture to be used as the view's texture.
     */
    void setTexture(LTexture *texture);

    /**
     * @brief Get the current LTexture used by the LTextureView.
     *
     * @return A pointer to the current LTexture used by the view.
     */
    virtual LTexture *texture() const;

    /**
     * @brief Enable or disable the custom destination size for the LTextureView.
     *
     * @param enabled True to enable custom destination size, false to disable.
     */
    virtual void enableDstSize(bool enabled);

    /**
     * @brief Check if the custom destination size is enabled for the LTextureView.
     *
     * @return True if custom destination size is enabled, false otherwise.
     */
    virtual bool dstSizeEnabled() const;

    /**
     * @brief Set the custom destination size of the LTextureView.
     *
     * @param w The width of the custom destination size.
     * @param h The height of the custom destination size.
     */
    virtual void setDstSize(Int32 w, Int32 h);

    /**
     * @brief Set the custom destination size of the LTextureView using an LSize object.
     *
     * @param dstSize The custom destination size as an LSize object.
     */
    void setDstSize(const LSize &dstSize);

    /**
     * @brief Enable or disable custom coloring for texture rendering.
     *
     * This function enables or disables custom coloring for the texture rendering process.
     * When custom coloring is enabled, the texture color is replaced by a custom color while preserving the texture's alpha channel.
     *
     * @param enabled A boolean value indicating whether custom coloring should be enabled (true) or disabled (false).
     */
    void enableCustomColor(bool enabled);

    /**
     * @brief Check if custom coloring for texture rendering is enabled.
     *
     * This function returns a boolean value indicating whether custom coloring is currently enabled for the texture rendering process.
     *
     * @return `true` if custom coloring is enabled, `false` otherwise.
     */
    bool customColorEnabled() const;

    /**
     * @brief Set a custom color for texture rendering while preserving the texture's alpha channel.
     *
     * This function sets a custom color for the texture rendering process, replacing the original texture color while keeping the texture's alpha channel intact.
     *
     * @param r The red component of the custom color (0.0 to 1.0).
     * @param g The green component of the custom color (0.0 to 1.0).
     * @param b The blue component of the custom color (0.0 to 1.0).
     */
    void setCustomColor(Float32 r, Float32 g, Float32 b);

    /**
     * @brief Set a custom color for texture rendering using an LRGBF object.
     *
     * This function sets a custom color for the texture rendering process, replacing the original texture color while keeping the texture's alpha channel intact.
     *
     * @param color The LRGBF object representing the custom color.
     */
    void setCustomColor(const LRGBF &color);

    /**
     * @brief Get the current custom color used for texture rendering.
     *
     * This function retrieves the current custom color that is being used for the texture rendering process. The custom color
     * replaces the original texture color while keeping the texture's alpha channel intact.
     *
     * @return A constant reference to the LRGBF object representing the current custom color.
     */
    const LRGBF &customColor() const;

    virtual bool nativeMapped() const override;
    virtual const LPoint &nativePos() const override;
    virtual const LSize &nativeSize() const override;
    virtual Int32 bufferScale() const override;
    virtual void enteredOutput(LOutput *output) override;
    virtual void leftOutput(LOutput *output) override;
    virtual const std::list<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damage() const override;
    virtual const LRegion *translucentRegion() const override;
    virtual const LRegion *opaqueRegion() const override;
    virtual const LRegion *inputRegion() const override;
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

    LPRIVATE_IMP(LTextureView)
};

#endif // LTEXTUREVIEW_H
