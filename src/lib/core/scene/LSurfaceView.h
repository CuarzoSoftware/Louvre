#ifndef LSURFACEVIEW_H
#define LSURFACEVIEW_H

#include <LView.h>
#include <LWeak.h>

/**
 * @brief View for displaying surfaces.
 *
 * The LSurfaceView class can be used to display client surfaces on a scene.
 * By default, it inherits the damage, input, translucent, and opaque regions, as well as the surface position
 * from the LSurface.
 *
 * To override any of these properties, you can use these specific methods:
 * - enableCustomPos(): Enables the customization of the surface position.
 * - enableCustomInputRegion(): Enables the customization of the input region.
 * - enableCustomTranslucentRegion(): Enables the customization of the translucent region.
 *
 * An LSurface can have multiple views, but only views with the primary() property set to true will clear its damage
 * when requestNextFrame() is called and also send LSurface::sendOutputEnterEvent() and LSurface::sendOutputLeaveEvent() to its client.
 * Use the setPrimary() method to set the primary property of a view.
 *
 * @attention The view should always be destroyed before its surface.
 */
class Louvre::LSurfaceView : public LView
{
public:
    /**
     * @brief LSurfaceView class constructor.
     *
     * @param surface Pointer to the LSurface associated with this view.
     * @param parent Pointer to the parent LView. Default value is nullptr.
     */
    LSurfaceView(LSurface *surface, LView *parent = nullptr) noexcept;

    LCLASS_NO_COPY(LSurfaceView)

    /**
     * @brief LSurfaceView class destructor.
     */
    ~LSurfaceView() noexcept;

    /**
     * @brief Gets the LSurface associated with the view.
     *
     * @return Pointer to the LSurface associated with the view.
     */
    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

    /**
     * @brief Check if the view is a primary view.
     *
     * Only primary views will clear their damage when requestNextFrame() is called.
     * The default value is true.
     *
     * @return True if the view is a primary view, false otherwise.
     */
    bool primary() const noexcept
    {
        return m_state.check(Primary);
    }

    /**
     * @brief Set the view as a primary view.
     *
     * Only primary views will clear their damage when requestNextFrame() is called.
     *
     * @param primary True to set the view as a primary view, false otherwise.
     */
    void setPrimary(bool primary) noexcept
    {
        m_state.setFlag(Primary, primary);
    }

    /**
     * @brief Check if custom position is enabled for the view.
     *
     * If enabled, nativePos() will return the position set with setCustomPos() instead of the surface position.
     * The default value is false.
     *
     * @return True if custom position is enabled, false otherwise.
     */
    bool customPosEnabled() const noexcept
    {
        return m_state.check(CustomPos);
    }

    /**
     * @brief Enable or disable custom position for the view.
     *
     * If enabled, nativePos() will return the position set with setCustomPos() instead of the surface position.
     *
     * @param enable True to enable custom position, false to disable.
     */
    void enableCustomPos(bool enable) noexcept
    {
        m_state.setFlag(CustomPos, enable);
    }

    /**
     * @brief Check if custom input region is enabled for the view.
     *
     * If enabled, inputRegion() will return the input region set with setCustomInputRegion() instead of the surface input region.
     * The default value is false.
     *
     * @return True if custom input region is enabled, false otherwise.
     */
    bool customInputRegionEnabled() const noexcept
    {
        return m_state.check(CustomInputRegion);
    }

    /**
     * @brief Enable or disable custom input region for the view.
     *
     * If enabled, inputRegion() will return the input region set with setCustomInputRegion() instead of the surface input region.
     *
     * @param enabled True to enable custom input region, false to disable.
     */
    void enableCustomInputRegion(bool enabled) noexcept
    {
        if (enabled == m_state.check(CustomInputRegion))
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_state.setFlag(CustomInputRegion, enabled);
    }

    /**
     * @brief Set a custom position for the view.
     *
     * This method allows you to set a custom position for the view, overriding the surface position.
     *
     * @param pos The custom position as an LPoint object.
     */
    void setCustomPos(const LPoint &pos) noexcept
    {
        setCustomPos(pos.x(), pos.y());
    }

    /**
     * @brief Set a custom position for the view.
     *
     * This method allows you to set a custom position for the view, overriding the surface position.
     *
     * @param x The X coordinate of the custom position.
     * @param y The Y coordinate of the custom position.
     */
    void setCustomPos(Int32 x, Int32 y) noexcept
    {
        if (x == m_customPos.x() && y == m_customPos.y())
            return;

        m_customPos.setX(x);
        m_customPos.setY(y);

        if (!repaintCalled() && customPosEnabled() && mapped())
            repaint();
    }

    /**
     * @brief Gets the custom position set for the view.
     *
     * @return The custom position as an LPoint object.
     */
    const LPoint &customPos() const noexcept
    {
        return m_customPos;
    }

    /**
     * @brief Set a custom input region for the view.
     *
     * This method allows you to set a custom input region for the view, overriding the surface input region.
     *
     * @param region Pointer to the custom input region as an LRegion object.
     */
    void setCustomInputRegion(const LRegion *region) noexcept
    {
        if (region)
        {
            if (m_customInputRegion)
                *m_customInputRegion = *region;
            else
                m_customInputRegion = std::make_unique<LRegion>(*region);
        }
        else
            m_customInputRegion.reset();
    }

    /**
     * @brief Gets the custom input region set for the view.
     *
     * @return Pointer to the custom input region as an LRegion object.
     */
    const LRegion *customInputRegion() const noexcept
    {
        return m_customInputRegion.get();
    }

    /**
     * @brief Enable or disable the custom translucent region for the view.
     *
     * If enabled, the view will use the custom translucent region set with setCustomTranslucentRegion()
     * instead of the surface translucent region.
     *
     * @param enabled True to enable the custom translucent region, false to disable.
     */
    void enableCustomTranslucentRegion(bool enabled) noexcept
    {
        if (enabled == customTranslucentRegionEnabled())
            return;

        m_state.setFlag(CustomTranslucentRegion, enabled);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Check if the custom translucent region is enabled for the view.
     *
     * @return True if the custom translucent region is enabled, false otherwise.
     */
    bool customTranslucentRegionEnabled() const noexcept
    {
        return m_state.check(CustomTranslucentRegion);
    }

    /**
     * @brief Set a custom translucent region for the view.
     *
     * This method allows you to set a custom translucent region for the view, overriding the surface translucent region.
     *
     * @param region Pointer to the custom translucent region as an LRegion object.
     */
    void setCustomTranslucentRegion(const LRegion *region) noexcept
    {
        if (region)
        {
            if (m_customTranslucentRegion)
                *m_customTranslucentRegion = *region;
            else
                m_customTranslucentRegion = std::make_unique<LRegion>(*region);
        }
        else
            m_customTranslucentRegion.reset();

        if (!repaintCalled() && customTranslucentRegionEnabled() && mapped())
            repaint();
    }

    /**
     * @brief Gets the source rect of the surface, equivalent to LSurface::srcRect().
     */
    const LRectF &srcRect() const;

    /**
     * @brief Ignore the LSurface::mapped() state.
     *
     * If enabled nativeMapped() will return `true` even if the surface is unmapped as long as it contains a texture.
     *
     * Disabled by default.
     */
    void enableAlwaysMapped(bool enabled) noexcept
    {
        if (enabled == alwaysMappedEnabled())
            return;

        const bool prev { mapped() };

        m_state.setFlag(AlwaysMapped, enabled);

        if (!repaintCalled() && prev != mapped())
            repaint();
    }

    /**
     * @see enableAlwaysMapped()
     */
    bool alwaysMappedEnabled() const noexcept
    {
        return m_state.check(AlwaysMapped);
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
    std::unique_ptr<LRegion> m_customInputRegion;
    std::unique_ptr<LRegion> m_customTranslucentRegion;
    std::vector<LOutput*> m_nonPrimaryOutputs;
    LWeak<LSurface> m_surface;
    LRectF m_tmpSrcRect { 0.f, 0.f, 1.f, 1.f};
    LPoint m_customPos;
};

#endif // LSURFACEVIEW_H
