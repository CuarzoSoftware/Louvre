#ifndef LSURFACEVIEW_H
#define LSURFACEVIEW_H

#include <LView.h>

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
 * @attention The view must always be destroyed before its surface.
 */
class Louvre::LSurfaceView : public LView
{
public:

    /// @cond OMIT
    LSurfaceView(const LSurfaceView&) = delete;
    LSurfaceView& operator= (const LSurfaceView&) = delete;
    /// @endcond

    /**
     * @brief LSurfaceView class constructor.
     *
     * @param surface Pointer to the LSurface associated with this view.
     * @param parent Pointer to the parent LView. Default value is nullptr.
     */
    LSurfaceView(LSurface *surface, LView *parent = nullptr);

    /**
     * @brief LSurfaceView class destructor.
     */
    ~LSurfaceView();

    /**
     * @brief Get the LSurface associated with the view.
     *
     * @return Pointer to the LSurface associated with the view.
     */
    LSurface *surface() const;

    /**
     * @brief Check if the view is a primary view.
     *
     * Only primary views will clear their damage when requestNextFrame() is called.
     * The default value is true.
     *
     * @return True if the view is a primary view, false otherwise.
     */
    virtual bool primary() const;

    /**
     * @brief Set the view as a primary view.
     *
     * Only primary views will clear their damage when requestNextFrame() is called.
     *
     * @param primary True to set the view as a primary view, false otherwise.
     */
    virtual void setPrimary(bool primary);

    /**
     * @brief Check if custom position is enabled for the view.
     *
     * If enabled, nativePos() will return the position set with setCustomPos() instead of the surface position.
     * The default value is false.
     *
     * @return True if custom position is enabled, false otherwise.
     */
    virtual bool customPosEnabled() const;

    /**
     * @brief Enable or disable custom position for the view.
     *
     * If enabled, nativePos() will return the position set with setCustomPos() instead of the surface position.
     *
     * @param enable True to enable custom position, false to disable.
     */
    virtual void enableCustomPos(bool enable);

    /**
     * @brief Check if custom input region is enabled for the view.
     *
     * If enabled, inputRegion() will return the input region set with setCustomInputRegion() instead of the surface input region.
     * The default value is false.
     *
     * @return True if custom input region is enabled, false otherwise.
     */
    virtual bool customInputRegionEnabled() const;

    /**
     * @brief Enable or disable custom input region for the view.
     *
     * If enabled, inputRegion() will return the input region set with setCustomInputRegion() instead of the surface input region.
     *
     * @param enabled True to enable custom input region, false to disable.
     */
    virtual void enableCustomInputRegion(bool enabled);

    /**
     * @brief Set a custom position for the view.
     *
     * This method allows you to set a custom position for the view, overriding the surface position.
     *
     * @param pos The custom position as an LPoint object.
     */
    void setCustomPos(const LPoint &pos);

    /**
     * @brief Set a custom position for the view.
     *
     * This method allows you to set a custom position for the view, overriding the surface position.
     *
     * @param x The X coordinate of the custom position.
     * @param y The Y coordinate of the custom position.
     */
    virtual void setCustomPos(Int32 x, Int32 y);

    /**
     * @brief Get the custom position set for the view.
     *
     * @return The custom position as an LPoint object.
     */
    virtual const LPoint &customPos() const;

    /**
     * @brief Set a custom input region for the view.
     *
     * This method allows you to set a custom input region for the view, overriding the surface input region.
     *
     * @param region Pointer to the custom input region as an LRegion object.
     */
    virtual void setCustomInputRegion(const LRegion *region);

    /**
     * @brief Get the custom input region set for the view.
     *
     * @return Pointer to the custom input region as an LRegion object.
     */
    virtual const LRegion *customInputRegion() const;

    /**
     * @brief Enable or disable the custom translucent region for the view.
     *
     * If enabled, the view will use the custom translucent region set with setCustomTranslucentRegion()
     * instead of the surface translucent region.
     *
     * @param enabled True to enable the custom translucent region, false to disable.
     */
    virtual void enableCustomTranslucentRegion(bool enabled);

    /**
     * @brief Check if the custom translucent region is enabled for the view.
     *
     * @return True if the custom translucent region is enabled, false otherwise.
     */
    virtual bool customTranslucentRegionEnabled() const;

    /**
     * @brief Set a custom translucent region for the view.
     *
     * This method allows you to set a custom translucent region for the view, overriding the surface translucent region.
     *
     * @param region Pointer to the custom translucent region as an LRegion object.
     */
    virtual void setCustomTranslucentRegion(const LRegion *region);

    const LRectF &srcRect() const;

    virtual bool nativeMapped() const override;
    virtual const LPoint &nativePos() const override;
    virtual const LSize &nativeSize() const override;
    virtual Float32 bufferScale() const override;
    virtual void enteredOutput(LOutput *output) override;
    virtual void leftOutput(LOutput *output) override;
    virtual const std::vector<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damage() const override;
    virtual const LRegion *translucentRegion() const override;
    virtual const LRegion *opaqueRegion() const override;
    virtual const LRegion *inputRegion() const override;
    virtual void paintEvent(const PaintEventParams &params) override;

    LPRIVATE_IMP_UNIQUE(LSurfaceView)
};

#endif // LSURFACEVIEW_H
