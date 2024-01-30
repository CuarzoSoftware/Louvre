#ifndef LSOLIDCOLORVIEW_H
#define LSOLIDCOLORVIEW_H

#include <LView.h>

/**
 * @brief View for displaying solid color rects.
 *
 * This class provides functionality to display solid color rectangles within a scene.
 */
class Louvre::LSolidColorView : public LView
{
public:

    /// @cond OMIT
    LSolidColorView(const LSolidColorView&) = delete;
    LSolidColorView& operator= (const LSolidColorView&) = delete;
    /// @endcond

    /**
     * @brief Construct a solid color view as a child of another view.
     *
     * @param parent The parent view that will contain this solid color view.
     */
    LSolidColorView(LView *parent);

    /**
     * @brief Construct a solid color view with the specified color components and alpha.
     *
     * @param r The red color component (0.0 to 1.0).
     * @param g The green color component (0.0 to 1.0).
     * @param b The blue color component (0.0 to 1.0).
     * @param a The alpha value (0.0 to 1.0).
     * @param parent The parent view that will contain this solid color view.
     */
    LSolidColorView(Float32 r = 0.f, Float32 g = 0.f, Float32 b = 0.f, Float32 a = 1.f, LView *parent = nullptr);

    /**
     * @brief Construct a solid color view with the specified color and alpha.
     *
     * @param color The color in RGB format.
     * @param a The alpha value (0.0 to 1.0).
     * @param parent The parent view that will contain this solid color view.
     */
    LSolidColorView(const LRGBF &color, Float32 a = 1.f, LView *parent = nullptr);

    /**
     * @brief Destructor for the solid color view.
     */
    ~LSolidColorView();

    /**
     * @brief Set the color of the solid color view.
     *
     * @param color The new color for the view in RGB format.
     */
    void setColor(const LRGBF &color);

    /**
     * @brief Set the color of the solid color view using individual RGB components.
     *
     * @param r The red color component (0.0 to 1.0).
     * @param g The green color component (0.0 to 1.0).
     * @param b The blue color component (0.0 to 1.0).
     */
    void setColor(Float32 r, Float32 g, Float32 b);

    /**
     * @brief Get the current color of the solid color view.
     *
     * @return The current color of the view in RGB format.
     */
    const LRGBF &color() const;

    /**
     * @brief Set the position of the solid color view.
     *
     * @param pos The new position of the view.
     */
    void setPos(const LPoint &pos);

    /**
     * @brief Set the position of the solid color view using individual X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    virtual void setPos(Int32 x, Int32 y);

    /**
     * @brief Set the size of the solid color view.
     *
     * @param size The new size of the view.
     */
    void setSize(const LSize &size);

    /**
     * @brief Set the size of the solid color view using width and height values.
     *
     * @param w The new width of the view.
     * @param h The new height of the view.
     */
    virtual void setSize(Int32 w, Int32 h);

    /**
     * @brief Set the input region for the solid color view.
     *
     * @param region The new input region for the view.
     */
    virtual void setInputRegion(const LRegion *region) const;

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

LPRIVATE_IMP_UNIQUE(LSolidColorView)
};

#endif // LSOLIDCOLORVIEW_H
