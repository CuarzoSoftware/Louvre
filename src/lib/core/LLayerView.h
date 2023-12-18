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
    LLayerView(LView *parent = nullptr);

    /**
     * @brief Destructor for LLayerView.
     */
    ~LLayerView();

    /**
     * @brief Set the position of the view.
     *
     * @param x The x-coordinate in surface coordinates.
     * @param y The y-coordinate in surface coordinates.
     */
    virtual void setPos(Int32 x, Int32 y);

    /**
     * @brief Set the size of the view.
     *
     * @param w The width in surface coordinates.
     * @param h The height in surface coordinates.
     */
    virtual void setSize(Int32 w, Int32 h);

    /**
     * @brief Set the input region for the view.
     *
     * @param region The input region to be set.
     */
    virtual void setInputRegion(const LRegion *region) const;

    /**
     * @brief Set the position of the view.
     *
     * @param pos The position as an LPoint in surface coordinates.
     */
    void setPos(const LPoint &pos);

    /**
     * @brief Set the size of the view.
     *
     * @param size The size as an LSize in surface coordinates.
     */
    void setSize(const LSize &size);

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
    virtual void paintRect(const PaintRectParams &params) override;

    LPRIVATE_IMP_UNIQUE(LLayerView)
};

#endif // LLAYERVIEW_H
