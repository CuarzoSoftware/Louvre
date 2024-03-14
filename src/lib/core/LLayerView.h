#ifndef LLAYERVIEW_H
#define LLAYERVIEW_H

#include <LView.h>

/**
 * @brief Container of views
 *
 * The LLayerView is a non-renderable LView. Unlike other views, it doesn't have content to render on its own.
 * Instead, it functions as a container for other views, allowing you to stack a group of views together or apply clipping to them.
 */
class Louvre::LLayerView : public LView
{
public:

    /// @cond OMIT
    LLayerView(const LLayerView&) = delete;
    LLayerView& operator= (const LLayerView&) = delete;
    /// @endcond

    /**
     * @brief Constructor for LLayerView.
     *
     * @param parent The parent view, if any.
     */
    inline LLayerView(LView *parent = nullptr) noexcept : LView(LView::Layer, false, parent) {}

    /**
     * @brief Destructor for LLayerView.
     */
    ~LLayerView() noexcept = default;

    /**
     * @brief Set the position of the view.
     *
     * @param x The x-coordinate in surface coordinates.
     * @param y The y-coordinate in surface coordinates.
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
     * @brief Set the position of the view.
     *
     * @param pos The position as an LPoint in surface coordinates.
     */
    inline void setPos(const LPoint &pos) noexcept
    {
        setPos(pos.x(), pos.y());
    }

    /**
     * @brief Set the size of the view.
     *
     * @param size The size as an LSize in surface coordinates.
     */
    inline void setSize(const LSize &size) noexcept
    {
        setSize(size.w(), size.h());
    }

    /**
     * @brief Set the size of the view.
     *
     * @param w The width in surface coordinates.
     * @param h The height in surface coordinates.
     */
    inline void setSize(Int32 w, Int32 h) noexcept
    {
        if (w == m_nativeSize.w() && h == m_nativeSize.h())
            return;

        m_nativeSize.setW(w);
        m_nativeSize.setH(h);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Set the input region for the view.
     *
     * @param region The input region to be set.
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
    std::vector<LOutput*> m_outputs;
    std::unique_ptr<LRegion> m_inputRegion;
    LPoint m_nativePos;
    LSize m_nativeSize { 256, 256 };
};

#endif // LLAYERVIEW_H
