#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>
#include <LWeak.h>

/**
 * @brief View for displaying textures
 *
 * The LTextureView class enables you to use an LTexture as a view within a scene.\n
 * You can set the view's texture using setTexture(), and passing `nullptr` unsets the texture, effectively unmapping the view.\n
 * Multiple views can share the same LTexture, and if the texture is destroyed, their texture() property is automatically set to `nullptr`.
 *
 * Texture views can also have a custom destination size, which may differ from their buffer size. In such cases, regions for damage, input, translucent,
 * and opaque must be defined based on the destination size.\n
 * To enable a custom destination size, use the enableDstSize() and setDstSize() methods.\n
 *
 * @note Using a custom destination size is recommended instead of relying on the scalingVector() option, as it allows for continued damage tracking
 *       within the scene.
 *
 * When destination size is disabled, the view size is by default equal to the texture size divided by its buffer scale if no other transformations
 * are applied.
 *
 * As of Louvre version 1.2.0, you can define a source rect with applied transformations, adhering to the behavior outlined in the
 * [Viewporter](https://wayland.app/protocols/viewporter) protocol.
 *
 * For additional methods and properties available, please refer to the documentation of the `LView` class.
 */
class Louvre::LTextureView : public LView
{
public:

    /**
     * @brief Construct an LTextureView with an optional LTexture and parent LView.
     *
     * @param texture The LTexture to be used as the view's texture. Default is nullptr.
     * @param parent The parent LView of the LTextureView. Default is nullptr.
     */
    LTextureView(LTexture *texture = nullptr, LView *parent = nullptr) noexcept : LView(LView::TextureType, true, parent)
    {
        m_texture.setOnDestroyCallback([this](auto)
        {
            updateDimensions();
            damageAll();
        });

        setTexture(texture);
    }

    LCLASS_NO_COPY(LTextureView)

    /**
     * @brief Destructor for the LTextureView.
     */
    ~LTextureView() noexcept = default;

    /**
     * @brief Set the position of the LTextureView.
     *
     * @param x The x-coordinate of the position.
     * @param y The y-coordinate of the position.
     */
    void setPos(Int32 x, Int32 y) noexcept
    {
        if (x == m_nativePos.x() && y == m_nativePos.y())
            return;

        m_nativePos.setX(x);
        m_nativePos.setY(y);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Set the position of the LTextureView using an LPoint object.
     *
     * @param pos The position as an LPoint object.
     */
    void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

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
     * @param scale The buffer scale factor.
     */
    void setBufferScale(Float32 scale) noexcept
    {
        if (scale < 0.25f)
            scale = 0.25f;

        if (scale == m_bufferScale)
            return;

        m_bufferScale = scale;
        updateDimensions();

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Set the LTexture for the LTextureView.
     *
     * @note If the current texture is destroyed, this property is automatically set to `nullptr`.
     *
     * @param texture The LTexture to be used as the view's texture.
     */
    void setTexture(LTexture *texture) noexcept;

    /**
     * @brief Gets the current LTexture used by the LTextureView.
     *
     * @return A pointer to the current LTexture used by the view.
     */
    LTexture *texture() const noexcept
    {
        return m_texture.get();
    }

    /**
     * @brief Enable or disable the custom destination size for the LTextureView.
     *
     * @param enabled True to enable custom destination size, false to disable.
     */
    void enableDstSize(bool enabled) noexcept
    {
        if (enabled != dstSizeEnabled())
        {
            m_state.setFlag(CustomDstSize, enabled);
            updateDimensions();
            repaint();
        }
    }

    /**
     * @brief Check if the custom destination size is enabled for the LTextureView.
     *
     * @return True if custom destination size is enabled, false otherwise.
     */
    bool dstSizeEnabled() const noexcept
    {
        return m_state.check(CustomDstSize);
    }

    /**
     * @brief Set the custom destination size of the LTextureView.
     *
     * @param w The width of the custom destination size.
     * @param h The height of the custom destination size.
     */
    void setDstSize(Int32 w, Int32 h) noexcept
    {
        if (w < 0)
            w = 0;

        if (h < 0)
            h = 0;

        if (w == m_customDstSize.w() && h == m_customDstSize.h())
            return;

        m_customDstSize.setW(w);
        m_customDstSize.setH(h);
        updateDimensions();

        if (!repaintCalled() && dstSizeEnabled() && mapped())
            repaint();
    }

    /**
     * @brief Set the custom destination size of the LTextureView using an LSize object.
     *
     * @param dstSize The custom destination size as an LSize object.
     */
    void setDstSize(const LSize &dstSize) noexcept
    {
        setDstSize(dstSize.w(), dstSize.h());
    }

    /**
     * @brief Enable or disable custom coloring for texture rendering.
     *
     * This method enables or disables custom coloring for the texture rendering process.
     * When custom coloring is enabled, the texture color is replaced by a custom color while preserving the texture's alpha channel.
     *
     * @param enabled A boolean value indicating whether custom coloring should be enabled (true) or disabled (false).
     */
    void enableCustomColor(bool enabled) noexcept
    {
        if (customColorEnabled() != enabled)
        {
            m_state.setFlag(CustomColor, enabled);
            damageAll();
        }
    }

    /**
     * @brief Check if custom coloring for texture rendering is enabled.
     *
     * This method returns a boolean value indicating whether custom coloring is currently enabled for the texture rendering process.
     *
     * @return `true` if custom coloring is enabled, `false` otherwise.
     */
    bool customColorEnabled() const noexcept
    {
        return m_state.check(CustomColor);
    }

    /**
     * @brief Set a custom color for texture rendering using an LRGBF object.
     *
     * This method sets a custom color for the texture rendering process, replacing the original texture color while keeping the texture's alpha channel intact.
     *
     * @param color The LRGBF object representing the custom color.
     */
    void setCustomColor(const LRGBF &color) noexcept
    {
        if (m_customColor == color)
            return;

        m_customColor = color;

        if (customColorEnabled())
            damageAll();
    }

    /**
     * @brief Gets the current custom color used for texture rendering.
     *
     * This method retrieves the current custom color that is being used for the texture rendering process. The custom color
     * replaces the original texture color while keeping the texture's alpha channel intact.
     *
     * @return A constant reference to the LRGBF object representing the current custom color.
     */
    const LRGBF &customColor() const noexcept
    {
        return m_customColor;
    }

    /**
     * @brief Enables or disables the use of a custom source rect.
     *
     * Enabling this feature allows you to specify a custom source rectangle using
     * setSrcRect().
     *
     * @note When disabled, the entire texture is used as the source.
     *
     * @param enabled If `true`, the custom source rectangle is enabled; if `false`, the entire
     *                texture is used as the source.
     */
    void enableSrcRect(bool enabled) noexcept
    {
        if (srcRectEnabled() == enabled)
            return;

        m_state.setFlag(CustomSrcRect, enabled);
        damageAll();
        updateDimensions();
    }

    /**
     * @brief Checks if the use of the source rect (srcRect()) is enabled.
     *
     * Disabled by default.
     *
     * @return `true` if enabled, `false` otherwise.
     *
     * @see enableSrcRect()
     * @see setSrcRect()
     */
    bool srcRectEnabled() const noexcept
    {
        return m_state.check(CustomSrcRect);
    }

    /**
     * @brief Sets the source rect of the texture to use.
     *
     * The source rect must be specified in surface coordinates, taking into
     * account the space generated after the inverse transform() (from transform() to LTransform::Normal) is applied.
     *
     * @note The source rect is used only if srcRectEnabled() is set to `true`.
     *
     * @param srcRect The source rect in surface coordinates.
     */
    void setSrcRect(const LRectF &srcRect) noexcept
    {
        if (m_customSrcRect == srcRect)
            return;

        m_customSrcRect = srcRect;
        updateDimensions();

        if (srcRectEnabled())
            damageAll();
    }

    /**
     * @brief Gets the source rect set with setSrcRect().
     *
     * @note When srcRectEnabled() returns `false` the rect covers the entire texture.
     *
     * @return A constant reference to the source rect specified using setSrcRect().
     */
    const LRectF &srcRect() const noexcept
    {
        return m_srcRect;
    }

    /**
     * @brief Sets the transform of the texture.
     *
     * Use this method to tell the scene the transformation that the texture's content **ALREADY HAS** and not the transformation you want to apply.
     *
     * For example, pass LTransform::Rotated90 if the texture's content is rotated 90 degrees **counter-clockwise**, and the scene
     * (if the current LOutput has a normal transform) will apply a 90-degree **clockwise** rotation to display it normally.
     *
     * Changing the transform affects how the source rectangle is defined and may also swap the original width
     * and height of the view when using transforms with 90 or 270 degrees rotation.
     *
     * @param transform The transform to be applied for proper display.
     */
    void setTransform(LTransform transform) noexcept
    {
        if (m_transform == transform)
            return;

        m_transform = transform;
        damageAll();
        updateDimensions();
    }

    /**
     * @brief Gets the transform set with setTransform().
     *
     * The default value is LTransform::Normal.
     */
    LTransform transform() const noexcept
    {
        return m_transform;
    }

    virtual bool nativeMapped() const noexcept override;
    virtual const LPoint &nativePos() const noexcept override;
    virtual const LSize &nativeSize() const noexcept override;
    virtual Float32 bufferScale() const noexcept override;
    virtual void enteredOutput(LOutput *output) noexcept override;
    virtual void leftOutput(LOutput *output) noexcept override;
    virtual const std::vector<LOutput*> &outputs() const noexcept override;
    virtual void requestNextFrame(LOutput *output) noexcept override;
    virtual const LRegion *damage() const noexcept override;
    virtual const LRegion *translucentRegion() const noexcept override;
    virtual const LRegion *opaqueRegion() const noexcept override;
    virtual const LRegion *inputRegion() const noexcept override;
    virtual void paintEvent(const PaintEventParams &params) noexcept override;

protected:
    LWeak<LTexture> m_texture;
    std::vector<LOutput*> m_outputs;
    std::unique_ptr<LRegion> m_inputRegion;
    std::unique_ptr<LRegion> m_translucentRegion;
    mutable LRectF m_customSrcRect {0.f, 0.f, 1.f, 1.f};
    mutable LRectF m_srcRect {0.f, 0.f, 1.f, 1.f};
    LRGBF m_customColor;
    LPoint m_nativePos;
    mutable LSize m_dstSize {1, 1};
    mutable LSize m_customDstSize {1, 1};
    Float32 m_bufferScale { 1.f };
    LTransform m_transform { LTransform::Normal };
    mutable UInt32 m_textureSerial { 0 };
    void updateDimensions() const noexcept;
};

#endif // LTEXTUREVIEW_H
