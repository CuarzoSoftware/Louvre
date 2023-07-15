#ifndef LVIEW_H
#define LVIEW_H

#include <LObject.h>

class Louvre::LView : public LObject
{
public:

    LView(UInt32 type, LView *parent = nullptr);
    virtual ~LView();

    /// Types of views included with Louvre
    enum Type : UInt32
    {
        Layer = 0,
        Surface = 1,
        Texture = 2,
        SolidColor = 3
    };

    LPainterMask *getMask(UInt32 slot) const;
    void setMask(UInt32 slot, LPainterMask *mask);

    /*!
     * Scene in which this view is currently embedded.
     * @returns nullptr if the view is not a child or subchild of an LScene
     */
    LScene *scene() const;

    /*!
     * A number used to identify the type of view, passed in the LView constructor.
     * Refer to LView::Type for additional information.
     */
    UInt32 type() const;

    /*!
     * Schedule a repaint for all outputs where this view
     * is currently visible (outputs returned by LView::outputs()).
     */
    void repaint();

    /*!
     * The parent of the view.
     * @returns nullptr if no parent is assigned to the view.
     */
    LView *parent() const;

    /*!
     * Sets the new parent for the view and inserts it at the end of it's children list.\n
     * Passing nullptr unsets the parent.
     */
    void setParent(LView *view);

    /*!
     * Inserts the surface after the prev view.\n
     * If switchParent is true, the view will be assigned the same parent as the prev view.\n
     * If switchParent is false, the view will only be reinserted if it shares the same parent with the prev view.\n
     * If prev is set to nullptr, the surface will be inserted at the beginning of its current parent children list,
     * regardless of the value of switchParent.
     */
    void insertAfter(LView *prev, bool switchParent = true);

    /*!
     * List of child views
     */
    std::list<LView*> &children() const;

    /*!
     * If true posC() returns nativePosC() + parent()->posC().\n
     * If false posC() returns nativePosC().\n
     * The default value is true.
     */
    bool parentOffsetEnabled() const;

    /*!
     * If enabled posC() will return nativePosC() + parent()->posC().\n
     * If disabled posC() will return nativePosC().
     */
    void enableParentOffset(bool enabled);

    /*!
     * The current view position.
     */
    const LPoint &posC() const;

    /*!
     * The current view size.
     */
    const LSize &sizeC() const;

    /*!
     * If true, child views will be clipped to the current view rectangle (posC(), sizeC()).
     */
    bool parentClippingEnabled() const;

    /*!
     * If enabled, child views will be clipped to the current view rectangle (posC(), sizeC()).
     */
    void enableParentClipping(bool enabled);

    /*!
     * If true, the view will receive pointer or touch events.
     */
    bool inputEnabled() const;

    /*!
     * If enabled, the view will receive pointer or touch events.
     */
    void enableInput(bool enabled);

    /*!
     * If true, sizeC() returns nativeSizeC() * scalingVector().\n
     * If false, sizeC() returns return nativeSizeC().
     */
    bool scalingEnabled() const;

    /*!
     * If enabled, sizeC() will return nativeSizeC() * scalingVector().\n
     * If disabled, sizeC() will return nativeSizeC().
     */
    void enableScaling(bool enabled);

    /*!
    * If enabled, children views will also be scaled with the scalingVector().
    */
    bool parentScalingEnabled() const;

    /*!
    * If enabled, children views will also be scaled with the scalingVector().
    */
    void enableParentScaling(bool enabled);

    /*!
     * The view size scaling vector.
     */
    const LSizeF &scalingVector(bool forceIgnoreParent = false) const;

    /*!
     * Sets the (w,h) scaling vector for the view size.\n
     * This only takes effect if the scalingVectorEnabled() propertry
     * returns true.\n
     * If enabled, the view size will be nativeSizeC() * scalingVector()
     */
    void setScalingVector(const LSizeF &scalingVector);

    /*!
     * This property does not directly indicate if the view will be rendered.
     * Use the mapped() property instead to check if the view will be rendered.
     */
    bool visible() const;

    /*!
     * Toggles the view visibility.\n
     * Enabling it does not guarantee that the view will be rendered; the nativeMapped() property
     * must return true for it to be considered mapped().\n
     * Disabling it, on the other hand, directly indicates that it is not mapped.
     */
    void setVisible(bool visible);

    /*!
     * Indicates whether the view should be rendered, taking into consideration
     * the nativeMapped() && visible() boolean operation.
     */
    bool mapped() const;

    /*!
     * The current view opacity without considering its parent opacity.
     */
    Float32 opacity(bool forceIgnoreParent = false) const;

    /*!
     * Sets the view opacity
     * @param opacity The opacity value in the range [0.0, 1.0]
     */
    void setOpacity(Float32 opacity);

    /*!
     * If true, the view opacity is multiplied by its parent opacity
     */
    bool parentOpacityEnabled() const;

    /*!
     * If enabled, the view opacity will be multiplied by its parent opacity
     */
    void enableParentOpacity(bool enabled);

    /*!
     * If true, requestNextFrame() is always called, even if the view
     * is not mapped or is occluded.\n
     * The default value is false.
     */
    bool forceRequestNextFrameEnabled() const;

    /*!
     * If enabled, requestNextFrame() is always called, even if the view
     * is not mapped or occluded.
     */
    void enableForceRequestNextFrame(bool enabled) const;

    /*!
     * Indicates whether the view should be rendered without taking
     * the visible() property into consideration.
     */
    virtual bool nativeMapped() const = 0;

    /*!
     * Position of the view without any transformations applied.
     */
    virtual const LPoint &nativePosC() const = 0;

    /*!
     * Size of the view without any transformations applied.
     */
    virtual const LSize &nativeSizeC() const = 0;

    /*!
     * Scale of the view buffer content.
     * This property is primarily used by the LSurfaceView and LTextureView types.
     * If a value less than or equal to 0 is returned, the global compositor scale is used.
     */
    virtual Int32 bufferScale() const = 0;

    /*!
     * Indicates that the view is visible on the given output.
     */
    virtual void enteredOutput(LOutput *output) = 0;

    /*!
     * Indicates that the view is no longer visible on the given output.
     */
    virtual void leftOutput(LOutput *output) = 0;

    /*!
     * Returns a list of outputs on which the view is currently visible.
     */
    virtual const std::list<LOutput*> &outputs() const = 0;

    /*!
     * This property indicates whether the view is itself renderable.
     * For example, all view types included in Louvre are renderable,
     * except for LLayerView, which serves as a container for other views
     * but does not produce any output by itself.
     */
    virtual bool isRenderable() const = 0;

    /*!
     * Notifies that the view is not occluded by other views and has been
     * rendered on the given output.
     * This should be used to clear the previous view damage or update its
     * content.
     * If forceRequestNextFrameEnabled() is true, this method is always called.
     */
    virtual void requestNextFrame(LOutput *output) = 0;

    /*!
     * Region within the view rect that need to be repainted.\n
     * The region rects are specified in compositor-local coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * The damage may be cleared after requestNextFrame() is called.\n
     * If nullptr is returned, the entire view rect will be
     * considered damaged.
     */
    virtual const LRegion *damageC() const = 0;

    /*!
     * Returns the translucent region within the view rectangle.\n
     * The region rects are specified in compositor-local coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If nullptr is returned, the entire view rect will be
     * considered translucent.
     */
    virtual const LRegion *translucentRegionC() const = 0;

    /*!
     * Returns the opaque region within the view rectangle.
     * The region rects are specified in compositor-local coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If nullptr is returned, the inverse of the translucent
     * region will be considered opaque.
     */
    virtual const LRegion *opaqueRegionC() const = 0;

    /*!
     * Region within the view rect that can receive input events (when the input property is enabled).\n
     * The region rects are specified in compositor-local coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If nullptr is returned, the entire view rect will receive input.
     */
    virtual const LRegion *inputRegionC() const = 0;

    /*!
     * Request to paint a rect of the view to the current output.
     */
    virtual void paintRectC(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha,
                           Int32 containerX, Int32 containerY) = 0;

LPRIVATE_IMP(LView)
};

#endif // LVIEW_H
