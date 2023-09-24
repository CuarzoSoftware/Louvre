#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>

/*!
 * @brief View for displaying textures
 *
 * The LTextureView allows you to use an LTexture as a view in a scene.
 * The used LTexture must remain valid while set. The same LTexture can be used in multiple views at a time.\n
 * To unset the texture, nullptr must be passed to setTexture(), which also unmaps the view.\n
 * LTextureViews can also have a custom destination size, which can differ from its buffer size. In that case, damage, input, translucent and opaque rigions
 * must be defined based on the dst size.\n
 * To enable a custom destination size, use the enableDstSize() and setDstSize() methods.\n
 * Using a custom dstSize() is recommended instead of using the scalingVector() option as damage tracking can still be used by the scene.\n
 * If dst size is disabled, the view size is equal to the texture size divided by its buffer scale.
 *
 * Please refer to the documentation of the LView class for additional methods and properties avaliable.
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
     * @param texture The LTexture to be used as the view's texture.
     */
    virtual void setTexture(LTexture *texture);

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

    /*!
     * @brief Enable or disable custom coloring for texture rendering.
     *
     * This function enables or disables custom coloring for the texture rendering process.
     * When custom coloring is enabled, the texture color is replaced by a custom color while preserving the texture's alpha channel.
     *
     * @param enabled A boolean value indicating whether custom coloring should be enabled (true) or disabled (false).
     */
    void enableCustomColor(bool enabled);

    /*!
     * @brief Check if custom coloring for texture rendering is enabled.
     *
     * This function returns a boolean value indicating whether custom coloring is currently enabled for the texture rendering process.
     *
     * @return `true` if custom coloring is enabled, `false` otherwise.
     */
    bool customColorEnabled() const;

    /*!
     * @brief Set a custom color for texture rendering while preserving the texture's alpha channel.
     *
     * This function sets a custom color for the texture rendering process, replacing the original texture color while keeping the texture's alpha channel intact.
     *
     * @param r The red component of the custom color (0.0 to 1.0).
     * @param g The green component of the custom color (0.0 to 1.0).
     * @param b The blue component of the custom color (0.0 to 1.0).
     */
    void setCustomColor(Float32 r, Float32 g, Float32 b);

    /*!
     * @brief Set a custom color for texture rendering using an LRGBF object.
     *
     * This function sets a custom color for the texture rendering process, replacing the original texture color while keeping the texture's alpha channel intact.
     *
     * @param color The LRGBF object representing the custom color.
     */
    void setCustomColor(const LRGBF &color);

    /*!
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
