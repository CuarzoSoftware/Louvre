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
    inline LSolidColorView(LView *parent = nullptr) noexcept :
        LView(LView::SolidColor, true, parent),
        m_color(0.f, 0.f, 0.f)
    {}

    /**
     * @brief Construct a solid color view with the specified color components and alpha.
     *
     * @param r The red color component (0.0 to 1.0).
     * @param g The green color component (0.0 to 1.0).
     * @param b The blue color component (0.0 to 1.0).
     * @param a The alpha value (0.0 to 1.0).
     * @param parent The parent view that will contain this solid color view.
     */
    inline LSolidColorView(Float32 r = 0.f, Float32 g = 0.f, Float32 b = 0.f, Float32 a = 1.f, LView *parent = nullptr) noexcept :
        LView(SolidColor, true, parent),
        m_color(r, g, b)
    {
        if (a < 0.f)
            a = 0.f;
        else if(a > 1.f)
            a = 1.f;

        m_opacity = a;
    }

    /**
     * @brief Construct a solid color view with the specified color and alpha.
     *
     * @param color The color in RGB format.
     * @param a The alpha value (0.0 to 1.0).
     * @param parent The parent view that will contain this solid color view.
     */
    inline LSolidColorView(const LRGBF &color, Float32 a = 1.f, LView *parent = nullptr) noexcept :
        LView(SolidColor, true, parent),
        m_color(color)
    {
        if (a < 0.f)
            a = 0.f;
        else if(a > 1.f)
            a = 1.f;

        m_opacity = a;
    }

    /**
     * @brief Destructor for the solid color view.
     */
    ~LSolidColorView() noexcept = default;

    /**
     * @brief Set the color of the solid color view.
     *
     * @param color The new color for the view in RGB format.
     */
    inline void setColor(const LRGBF &color) noexcept
    {
        if (m_color.r != color.r || m_color.g != color.g || m_color.b != color.b)
        {
            m_color = color;
            markAsChangedOrder(false);
            repaint();
        }
    }

    /**
     * @brief Get the current color of the solid color view.
     *
     * @return The current color of the view in RGB format.
     */
    inline const LRGBF &color() const noexcept
    {
        return m_color;
    }

    /**
     * @brief Set the position of the solid color view.
     *
     * @param pos The new position of the view.
     */
    inline void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

    /**
     * @brief Set the position of the solid color view using individual X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    inline void setPos(Int32 x, Int32 y) noexcept
    {
        if (x == m_nativePos.x() && y == m_nativePos.y())
            return;

        m_nativePos.setX(x);
        m_nativePos.setY(y);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Set the size of the solid color view.
     *
     * @param size The new size of the view.
     */
    inline void setSize(const LSize &size) noexcept
    {
        setSize(size.w(), size.h());
    }

    /**
     * @brief Set the size of the solid color view using width and height values.
     *
     * @param w The new width of the view.
     * @param h The new height of the view.
     */
    inline void setSize(Int32 w, Int32 h) noexcept
    {
        if (w != m_nativeSize.w() || h != m_nativeSize.h())
        {
            m_nativeSize.setW(w);
            m_nativeSize.setH(h);

            if (!repaintCalled() && mapped())
                repaint();
        }
    }

    /**
     * @brief Set the input region for the solid color view.
     *
     * @param region The new input region for the view.
     */
    inline void setInputRegion(const LRegion *region) noexcept
    {
        if (region)
        {
            if (m_inputRegion)
                *m_inputRegion = *region;
            else
                m_inputRegion = std::make_unique<LRegion>(*region);
        }
        else
            m_inputRegion.reset();
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
    std::vector<LOutput *> m_outputs;
    std::unique_ptr<LRegion> m_inputRegion;
    LRGBF m_color;
    LPoint m_nativePos;
    LSize m_nativeSize;
};

#endif // LSOLIDCOLORVIEW_H
