#ifndef LVIEW_H
#define LVIEW_H

#include <LObject.h>
#include <LPointer.h>
#include <LFramebuffer.h>

/**
 * @brief Base class for LScene views.
 *
 * The LView class provides a base interface for creating views that can be shown in an LScene.\n
 * This class should be used for creating custom views that are different from those shipped with the library.\n
 *
 * Louvre provides a pre-made set of views for most common purposes:
 *
 * * LLayerView: This view is used only as a container for other views and is not renderable on its own.
 * * LSurfaceView: This view can be used to display client surfaces.
 * * LTextureView: This view can be used to display textures.
 * * LSolidColorView: This view can be used to display solid color rectangles.
 * * LSceneView: This view is a scene on its own, and its content is rendered in its own framebuffer.
 *
 * ## Stacking
 *
 * Each view can have a parent and multiple children. This hierarchy and the order given by their children views list
 * determine the stacking order in which they are rendered.\n
 * Children views are always stacked on top of its parent, and the last view in a children list is the one stacked on top the rest.
 *
 * ## Creating custom views
 *
 * To create a custom view, you must implement several virtual methods. In summary, for an LScene or LSceneView to be able to render an LView,
 * it must provide the following information:
 *
 * ### Position
 *
 * The position of the view in surface coordinates must be returned by the nativePos() virtual method.\n
 * This method is used by the pos() method, which returns the position equal to nativePos(), or transformed
 * if parent offset or parent scaling is enabled.
 *
 * ### Size
 *
 * The size of the view in surface coordinates should be implemented in the nativeSize() virtual method.
 * This size is then returned by the size() method if no transformation is applied, or may differ if scaling or parent scaling is enabled.
 *
 * ### Mapping
 *
 * To tell a scene if the view is visible, the nativeMapped() bool method must be implemented.
 * This is then returned by the mapped() method, which may differ from nativeMapped() if the visible() property is different, or if the
 * view has no parent, or if its parent is mapped.
 * In summary, a view can only be mapped if nativeMapped() is `true`, the visible() property is `true`, it has a parent, and the parent is also mapped.
 *
 * ### Renderable
 *
 * If the view has content that can be rendered, for example, a texture, then the isRenderable() method must return `true`. On the other hand, if
 * the view is simply a container for other views and is not renderable on its own, then it should return `false`.
 *
 * ### Intersected Outputs
 *
 * The outputs() virtual method must return a list with the outputs where the view is currently visible. When the view changes, repaint() is called,
 * and all those intersected outputs are scheduled for repainting.
 * The scene informs which outputs a view is currently on based on its rect through the enteredOutput() and leftOutput() virtual methods.
 * So, those methods must be used to update the intersected outputs list.
 *
 * ### Painting
 *
 * If the view is renderable, the paintEvent() method must be implemented.
 * This method is called by the scene to request the view to render itself into the current framebuffer.
 * It provides an LPainter, the region of the view to be rendered in global compositor coordinates, and a boolean variable indicating whether blending
 * is enabled.
 * The paintEvent() method is called twice per frame: once for drawing only the opaque regions of the view (blending disabled), and a second pass for
 * the translucent regions (blending enabled).
 *
 * ### Damage
 *
 * To inform the scene about which rects within the view must be re-rendered (due to changes), the damage() virtual method must return an LRegion containing
 * the damaged rects. If `nullptr` is returned, the entire view is considered damaged.\n
 * The scene automatically damages the entire view if its position, size, or stacking order changes from one frame to another, so it's not necessary to
 * add damage in those cases.
 * Usually, the damage should be cleared once the view has been rendered. To determine if a view was rendered on a specific output, the requestNextFrame() method
 * is invoked by the scene. So within that method, the damage should be cleared.
 *
 * ### Opaque and Translucent region
 *
 * To prevent the scene view from rendering views occluded by opaque regions, the translucentRegion() and opaqueRegion() must be implemented.
 * They must return an LRegion with the translucent and opaque regions, respectively, in surface coordinates.\n
 * If translucentRegion() returns `nullptr`, the entire view is considered translucent. On the other hand, if opaqueRegion is `nullptr`,
 * the opaque region is considered the inverse of the translucentRegion().
 * So normally, it's only needed to specify the translucent region. The opaque region must be specified if it has been calculated before
 * so that the scene can use that information and prevent re-calculating the inverse translucent region.
 *
 * ### Input region
 *
 * To define which parts of the view can receive pointer or touch events, the inputRegion() must be implemented.
 * If `nullptr` is returned, the entire view is considered capable of receiving events.
 *
 * ## Input events
 *
 * Scenes can trigger specific input events on views through the pointerEnterEvent(), pointerMoveEvent(), keyEvent(), and other related methods.
 * Implement these virtual methods to listen and respond to those events if needed.
 */
class Louvre::LView : public LObject
{
public:

    /**
     * @brief Parameters used within a paintEvent().
     */
    struct PaintEventParams
    {
        /// LPainter object to perform the painting.
        LPainter *painter;

        /// Region to draw in global compositor coordinates.
        LRegion *region;

        /// Indicates if the region to render is opaque (false) or translucent (true)
        bool blending;
    };

    /**
     * @brief Construct an LView object.
     *
     * @param type Type ID of the view, such as those listed in LView::Type.
     * @param parent Parent view.
     */
    LView(UInt32 type, LView *parent = nullptr);

    /// @cond OMIT
    LView(const LView&) = delete;
    LView& operator= (const LView&) = delete;
    /// @endcond

    /**
     * @brief Destructor for the LView class.
     */
    virtual ~LView();

    /// Types of views included with Louvre
    enum Type : UInt32
    {
        /// LLayerView
        Layer = 0,

        /// LSurfaceView
        Surface = 1,

        /// LTextureView
        Texture = 2,

        /// LSolidColorView
        SolidColor = 3,

        /// LSceneView
        Scene = 4
    };

    void damageAll();

    /**
     * @brief Get the scene in which this view is currently embedded.
     *
     * @returns A pointer to the scene that contains this view, or `nullptr` if the view is not part of any scene.
     */
    LScene* scene() const;

    /**
     * @brief Get the LSceneView in which this view is currently embedded.
     *
     * @returns A pointer to the LSceneView that contains this view, or `nullptr` if the view is not part of any LSceneView.
     */
    LSceneView* parentSceneView() const;

    /**
     * @brief Get the identifier for the type of view.
     *
     * This method returns a number used to identify the type of view that was passed in the LView constructor.\n
     * For additional information about view types, refer to the LView::Type enumeration.
     *
     * @returns The identifier representing the type of view.
     */
    UInt32 type() const;

    /**
     * @brief Schedule a repaint for all outputs where this view is currently visible.
     *
     * This method triggers a repaint for all outputs where this view is currently visible.\n
     * Outputs are those returned by the LView::outputs() method.
     */
    void repaint();

    /**
     * @brief Get the parent of the view.
     *
     * @returns A pointer to the parent view, or `nullptr` if no parent is assigned to the view.
     */
    LView* parent() const;

    /**
     * @brief Set the new parent for the view and insert it at the end of its children list.
     *
     * This method sets the new parent for the view and inserts it at the end of its parent's children list.
     * If 'view' is set to `nullptr`, the parent is unset, and the view is unmapped.
     *
     * @param view The new parent view to be set.
     */
    void setParent(LView* view);

    /**
     * @brief Insert the view after the 'prev' view.
     *
     * This method inserts the view after the 'prev' view in the parent's children list.
     * If 'switchParent' is `true`, the view will be assigned the same parent as the 'prev' view.
     * If 'switchParent' is `false`, the view will only be reinserted if it shares the same parent with the 'prev' view.
     * If 'prev' is set to `nullptr`, the view will be inserted at the beginning of its current parent's children list,
     * regardless of the value of 'switchParent'.
     *
     * @param prev The view after which this view will be inserted.
     * @param switchParent If `true`, the view will be assigned the same parent as the 'prev' view.
     */
    void insertAfter(LView* prev, bool switchParent = true);

    /**
     * @brief Get the list of child views.
     *
     * This method returns a reference to the list of child views of the current view.
     *
     * @returns A reference to the list of child views.
     */
    std::list<LView*>& children() const;

    /**
     * @brief Check if the parent's offset is applied to the view position.
     *
     * If this method returns `true`, the position returned by pos() includes the parent's offset (parent()->pos()).
     *
     * The default value is `true`.
     *
     * @returns `true` if the parent's offset is applied to the view position, `false` otherwise.
     */
    bool parentOffsetEnabled() const;

    /**
     * @brief Enable or disable the parent's offset for the view position.
     *
     * If enabled, the position returned by pos() will include the parent's offset (parent()->pos()).
     *
     * The default value is `true`.
     *
     * @param enabled If `true`, the parent's offset will be applied to the view position.
     */
    void enableParentOffset(bool enabled);

    /**
     * @brief Get the current position of the view with applied transformations.
     *
     * This method returns the current position of the view with any applied transformations.
     *
     * @returns The position of the view.
     */
    const LPoint& pos() const;

    /**
     * @brief Get the current size of the view with applied transformations.
     *
     * This method returns the current size of the view with any applied transformations.
     *
     * @returns The size of the view.
     */
    const LSize& size() const;

    /**
     * @brief Check if the view is currently being clipped to the clippingRect() property.
     *
     * This method returns `true` if the view is currently being clipped to the specified
     * clipping rectangle, which is defined by the clippingRect() property. If the view is not
     * clipped to the clipping rectangle, the method returns `false`.
     *
     * The default value is `false`.
     *
     * @returns `true` if the view is being clipped to the clipping rectangle, `false` otherwise.
     */
    bool clippingEnabled() const;

    /**
     * @brief Enable or disable clipping of the view to the clippingRect() property.
     *
     * If enabled, the view will be clipped to the current clipping rectangle defined by the
     * clippingRect() property. If disabled, the view will not be clipped, allowing its entire
     * content to be visible.
     *
     * The default value is `false`.
     *
     * @param enabled If `true`, the view will be clipped to the clippingRect() property.
     *                If `false`, clipping will be disabled, and the full view will be visible.
     */
    void enableClipping(bool enabled);

    /**
     * @brief Get the current clipping rectangle defined by the clippingRect() property.
     *
     * This method returns a constant reference to the current clipping rectangle
     * that is being used to clip the view. The clipping rectangle is defined by the
     * clippingRect() property.
     *
     * @returns A constant reference to the current clipping rectangle.
     */
    const LRect &clippingRect() const;

    /**
     * @brief Set the clipping rectangle for the view using the clippingRect() property.
     *
     * This method sets the clipping rectangle for the view using the clippingRect() property.
     * When clipping is enabled, the view's content outside this rectangle will be clipped
     * (not visible).
     *
     * @note The rect is not local to the view's position, if you need to clip the view without having to
     *       update the rect pos each time the view moves, use parent clipping instead.
     *
     * @param rect The clipping rectangle to set for the view using the clippingRect() property.
     */
    void setClippingRect(const LRect &rect);

    /**
     * @brief Check if the view clipping to the current parent view rect is enabled.
     *
     * This method returns `true` if the view clipping to the current parent view rect is enabled, `false` otherwise.
     *
     * The default value is `false`.
     *
     * @returns `true` if the view is clipped to the current parent view rect, `false` otherwise.
     */
    bool parentClippingEnabled() const;

    /**
     * @brief Enable or disable clipping of the view to the current parent view rect.
     *
     * If enabled, the view will be clipped to the current parent view rect.
     *
     * The default value is `false`.
     *
     * @param enabled If `true`, the view will be clipped to the current parent view rect.
     */
    void enableParentClipping(bool enabled);

    /**
     * @brief Check if the view receives pointer and touch events.
     *
     * This method returns `true` if the view receives pointer and touch events.
     * However, keyboard events are always received regardless of this setting.
     *
     * @returns `true` if the view receives pointer and touch events, `false` otherwise.
     */
    bool inputEnabled() const;

    /**
     * @brief Enable or disable pointer and touch events for the view.
     *
     * If enabled, the view will receive pointer and touch events.
     * However, keyboard events are always received regardless of this setting.
     *
     * @param enabled If `true`, the view will receive pointer and touch events.
     */
    void enableInput(bool enabled);

    /**
     * @brief Check if scaling is enabled for the view's size.
     *
     * This method returns `true` if the view's size is scaled using the scalingVector(), `false` otherwise.
     *
     * The default value is `false`.
     *
     * @returns `true` if the view's size is scaled, `false` otherwise.
     */
    bool scalingEnabled() const;

    /**
     * @brief Enable or disable scaling for the view's size.
     *
     * If enabled, the view's size will be scaled using the scalingVector().
     *
     * Disabled by default.
     *
     * @param enabled If `true`, the view's size will be scaled.
     */
    void enableScaling(bool enabled);

    /**
     * @brief Check if the size and position are scaled by the parent scaling vector.
     *
     * This method returns `true` if the view's size and position are scaled by the parent's scaling vector, `false` otherwise.
     *
     * The default value is `false`.
     *
     * @returns `true` if the size and position are scaled by the parent's scaling vector, `false` otherwise.
     */
    bool parentScalingEnabled() const;

    /**
     * @brief Enable or disable scaling of the size and position by the parent's scaling vector.
     *
     * If enabled, the view's size and position will be scaled by the parent's scaling vector.
     *
     * The default value is `false`.
     *
     * @param enabled If `true`, the view's size and position will be scaled by the parent's scaling vector.
     */
    void enableParentScaling(bool enabled);

    /**
     * @brief Get the scaling vector for the view's size.
     *
     * This method returns the scaling vector for the view's size.
     *
     * @param forceIgnoreParent If set to `false`, the vector is multiplied by the parent scaling vector.
     * @returns The scaling vector for the view's size.
     */
    const LSizeF& scalingVector(bool forceIgnoreParent = false) const;

    /**
     * @brief Set the scaling vector for the view's size.
     *
     * If scalingEnabled() returns `true`, the view's size will be scaled using the provided scaling vector (nativeSize() * scalingVector()).
     * Setting a value to 1 disables scaling for that axis.
     *
     * @param scalingVector The (width, height) scaling vector for the view's size.
     * @warning Scaling should be used with moderation, preferably during animations, as damage tracking is disabled due to precision loss caused by scaling.
     * This means the entire view is repainted if changes occur, which can be inefficient.
     */
    void setScalingVector(const LSizeF& scalingVector);

    /**
     * @brief Check if the view is marked as visible.
     *
     * This method indicates whether the view is marked as visible. However, it does not directly indicate if the view will be rendered.
     * To check if the view will be rendered, use the mapped() property instead.
     *
     * @returns `true` if the view is marked as visible, `false` otherwise.
     */
    bool visible() const;

    /**
     * @brief Toggle the view visibility.
     *
     * Enabling visibility does not guarantee that the view will be rendered. On the other hand, disabling it directly indicates that it is not mapped and will not be rendered.
     *
     * @see mapped()
     *
     * @param visible If `true`, the view will be marked as visible, if `false`, it will be marked as not visible.
     */
    void setVisible(bool visible);

    /**
     * @brief Check if the view should be rendered, taking into consideration several conditions.
     *
     * This method indicates whether the view should be rendered, considering the nativeMapped() && visible() && parent() && parent()->mapped() boolean operation.
     *
     * @returns `true` if the view should be rendered, `false` otherwise.
     */
    bool mapped() const;

    /**
     * @brief Get the current view opacity.
     *
     * This method returns the current view opacity.
     *
     * @param forceIgnoreParent If set to `false`, the opacity is multiplied by the parent's opacity.
     * @returns The view's opacity value in the range [0.0, 1.0].
     */
    Float32 opacity(bool forceIgnoreParent = false) const;

    /**
     * @brief Set the view opacity.
     *
     * This method sets the view's opacity. Setting the value to 1.0 disables opacity.
     *
     * @warning Opacity should be used with moderation, for example, only on animations, as the opaqueRegion() is not considered in the scenario, so the content behind the view is always repainted when the view changes.
     *
     * @param opacity The opacity value in the range [0.0, 1.0].
     */
    void setOpacity(Float32 opacity);

    /**
     * @brief Check if the view's opacity is multiplied by its parent's opacity.
     *
     * This method returns `true` if the view's opacity is multiplied by its parent's opacity, `false` otherwise.
     *
     * The default value is `true`.
     *
     * @returns `true` if the view's opacity is multiplied by its parent's opacity, `false` otherwise.
     */
    bool parentOpacityEnabled() const;

    /**
     * @brief Enable or disable the view's opacity being multiplied by its parent's opacity.
     *
     * If enabled, the view's opacity will be multiplied by its parent's opacity.
     *
     * The default value is `true`.
     *
     * @param enabled If `true`, the view's opacity will be multiplied by its parent's opacity, if `false`, it will not be affected by the parent's opacity.
     */
    void enableParentOpacity(bool enabled);

    /**
     * @brief Check if the requestNextFrame() is enabled.
     *
     * If this method returns `true`, requestNextFrame() will be called even if the view
     * is occluded by other views or not mapped.
     *
     * @return `true` if requestNextFrame() is forced to be called, otherwise, `false`.
     */
    bool forceRequestNextFrameEnabled() const;

    /**
     * @brief Enable or disable the requestNextFrame() to be called always.
     *
     * When enabled, requestNextFrame() will be called even if the view
     * is occluded by other view or not mapped.
     *
     * @param enabled `true` to enable requestNextFrame(), `false` to disable.
     */
    void enableForceRequestNextFrame(bool enabled) const;

    /**
     * @brief Set the alpha blending function for the view.
     *
     * This method sets the OpenGL blend function for the view. Refer to the documentation
     * of [glBlendFuncSeparate()](https://docs.gl/es2/glBlendFuncSeparate) for more information.
     *
     * @note This only works when the autoBlendFuncEnabled() property is disabled.
     *
     * @param sRGBFactor Source RGB factor for blending.
     * @param dRGBFactor Destination RGB factor for blending.
     * @param sAlphaFactor Source alpha factor for blending.
     * @param dAlphaFactor Destination alpha factor for blending.
     */
    void setBlendFunc(GLenum sRGBFactor, GLenum dRGBFactor, GLenum sAlphaFactor, GLenum dAlphaFactor);

    /**
     * @brief Enable or disable automatic blend function adjustment.
     *
     * When the automatic blend function is enabled, the blend function dynamically adjusts based on whether rendering occurs
     * in an output framebuffer or a custom framebuffer (e.g., an LRenderBuffer or LSceneView).
     *
     * By default, automatic blend function adjustment is enabled. When enabled, the blend function set with setBlendFunc() is ignored.
     *
     * @param enabled `true` to enable automatic blend function adjustment, `false` to disable.
     */
    void enableAutoBlendFunc(bool enabled);

    /**
     * @brief Check whether the automatic blend function adjustment is enabled.
     *
     * @return `true` if automatic blend function adjustment is enabled, `false` otherwise.
     */
    bool autoBlendFuncEnabled() const;

    /**
     * @brief Set the color factor.
     *
     * This method allows you to set a color factor that influences the resulting color of every painting operation.
     * By default, the color factor is (1.0, 1.0, 1.0, 1.0), which has no effect on the colors.
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a);

    /**
     * @brief Get the color factor.
     *
     * This method returns the current color factor of the view set with setColorFactor().
     */
    const LRGBAF &colorFactor();

    /**
     * @brief Checks if the pointer/cursor is inside the view's input region.
     *
     * @warning Even if the pointer is over the input region it may return `false` if another view with block pointer enabled is in front.\n
     *
     * @return `true` if the pointer/cursor is inside the input region; otherwise, `false`.
     */
    bool pointerIsOver() const;

    /**
     * @brief Enable or disable blocking of pointer or touch events to views behind the view's input region.
     *
     * If set to `true`, pointer or touch events will not be sent to views behind the view's input region.
     *
     * @param enabled `true` to enable blocking; `false` to disable.
     */
    void enableBlockPointer(bool enabled);

    /**
     * @brief Checks if blocking of pointer or touch events to views behind the view's input region is enabled.
     *
     * @return `true` if blocking is enabled; otherwise, `false`.
     */
    bool blockPointerEnabled() const;

    /**
     * @brief Get the bounding box of the view and all its mapped children.
     *
     * This method returns a box containing the view and all its mapped children, even if the children
     * are outside or clipped by the view's rect.
     *
     * @return The bounding box of the view and its mapped children.
     */
    LBox boundingBox() const;

    /**
     * @brief Tells whether the view should be rendered.
     *
     * @return `true` if the view should be rendered without considering visible(), otherwise `false`.
     */
    virtual bool nativeMapped() const = 0;

    /**
     * @brief Get the position of the view without any transformations applied.
     *
     * Must return the position of the view in surface coordinates.
     *
     * @return The position of the view as an LPoint object.
     */
    virtual const LPoint &nativePos() const = 0;

    /**
     * @brief Get the size of the view without any transformations applied.
     *
     * Must return the size of the view in surface coordinates.
     *
     * @return The size of the view as an LSize object.
     */
    virtual const LSize &nativeSize() const = 0;

    /**
     * @brief Get the scale of the view buffer content.
     *
     * This property is primarily used by views that contain a buffer like for example the LSceneView, LSurfaceView and LTextureView types.
     *
     * @return The buffer scale as an Int32 value.
     */
    virtual Float32 bufferScale() const = 0;

    /**
     * @brief Indicate that the view is visible on the given output.
     *
     * This method is invoked by a LScene when the view's rect intersects an output.
     *
     * @param output The LOutput where the view is visible.
     */
    virtual void enteredOutput(LOutput *output) = 0;

    /**
     * @brief Indicate that the view is no longer visible on the given output.
     *
     * This method is invoked by a LScene when the view's rect no longer intersects an output.
     *
     * @param output The LOutput from which the view is no longer visible.
     */
    virtual void leftOutput(LOutput *output) = 0;

    /**
     * @brief Get a vector of output pointers on which the view is currently visible.
     *
     * Must return a vector of output pointers where the view is currently visible.
     * Use the enteredOutput() and leftOutput() methods to update the vector.
     *
     * @return A reference to a vector of LOutput pointers representing the outputs where the view is visible.
     */
    virtual const std::vector<LOutput*> &outputs() const = 0;

    /**
     * @brief Check if the view is itself renderable.
     *
     * This property indicates whether the view is capable of rendering its content (check paintEvent()).
     * For example, all view types included in Louvre are renderable,
     * except for LLayerView, which serves as a container for other views
     * but does not produce any output by itself.
     *
     * @return `true` if the view is renderable; otherwise, `false`.
     */
    virtual bool isRenderable() const = 0;

    /**
     * @brief Notify that the view has been rendered on the given output.
     *
     * This method is called by LScene and should be used to clear the previous view damage or update its content.
     * If forceRequestNextFrameEnabled() is `true`, this method is always called.
     *
     * @param output The LOutput on which the view is rendered.
     */
    virtual void requestNextFrame(LOutput *output) = 0;

    /**
     * @brief Get the region within the view rect that needs to be repainted.
     *
     * The region rects are specified in surface coordinates within the view,
     * without any scaling, clipping, or offset transformations applied.
     * The damage may be cleared after requestNextFrame() is called.
     * If `nullptr` is returned, the entire view rect will be considered damaged.
     * If the view has no damage, simply pass an empty LRegion (not `nullptr`).
     *
     * @return A pointer to the LRegion specifying the damaged area within the view,
     *         or `nullptr` if the entire view rect is damaged.
     */
    virtual const LRegion *damage() const = 0;

    /**
     * Returns the translucent region within the view rectangle.\n
     * The region rects are specified in surface coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If `nullptr` is returned, the entire view rect will be
     * considered translucent.
     */
    virtual const LRegion *translucentRegion() const = 0;

    /**
     * Returns the opaque region within the view rectangle.
     * The region rects are specified in surface coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If `nullptr` is returned, the inverse of the translucent
     * region will be considered opaque.
     */
    virtual const LRegion *opaqueRegion() const = 0;

    /**
     * Region within the view rect that can receive input events (when the inputEnabled() property is enabled).\n
     * The region rects are specified in surface coordinates within the view,
     * without any scaling, clipping, or offset transformations.\n
     * If `nullptr` is returned, the entire view rect will receive input.
     */
    virtual const LRegion *inputRegion() const = 0;

    /**
     * @brief Request to paint a region of the view to the current framebuffer.
     *
     * This method is used by LSceneView to request the view to paint a specified region
     * on the current framebuffer. The painting is performed using the provided LPainter object.
     *
     * @note Alternatively, you have the option to use your own custom OpenGL shaders/program for rendering, in place of the provided LPainter.
     */
    virtual void paintEvent(const PaintEventParams &params) = 0;

    /**
     * @brief Handle the pointer enter event within the view.
     *
     * @param localPos The local position of the pointer within the view.
     */
    virtual void pointerEnterEvent(const LPoint &localPos);

    /**
     * @brief Handle the pointer move event within the view.
     *
     * This event is only called if pointerEnterEvent() was called before, and therefore when pointerIsOver() returns `true`.
     *
     * @param localPos The local position of the pointer within the view.
     */
    virtual void pointerMoveEvent(const LPoint &localPos);

    /**
     * @brief Handle the pointer leave event within the view.
     */
    virtual void pointerLeaveEvent();

    /**
     * @brief Handle the pointer button event within the view.
     *
     * This event is only called if pointerEnterEvent() was called before, and therefore when pointerIsOver() returns `true`.
     *
     * @param button The button that triggered the event (e.g., left button, right button, etc.).
     * @param state The state of the button (e.g., pressed, released, etc.).
     */
    virtual void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state);

    /**
     * @brief Handle the pointer axis event within the view.
     *
     * This event is only called if pointerEnterEvent() was called before, and therefore when pointerIsOver() returns `true`.
     *
     * @param axisX The x-coordinate of the axis movement.
     * @param axisY The y-coordinate of the axis movement.
     * @param discreteX The discrete x-coordinate of the axis movement.
     * @param discreteY The discrete y-coordinate of the axis movement.
     * @param source The source of the axis event (e.g., mouse wheel, touchpad, etc.).
     */
    virtual void pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source);

    /**
     * @brief Handle the key modifiers event within the view.
     *
     * Keyboard events are allways called, even if inputEnabled() is set to `false`.
     *
     * @param depressed The state of the depressed key modifiers.
     * @param latched The state of the latched key modifiers.
     * @param locked The state of the locked key modifiers.
     * @param group The key group state.
     */
    virtual void keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);

    /**
     * @brief Handle the key event within the view.
     *
     * Keyboard events are allways called, even if inputEnabled() is set to `false`.
     *
     * @param keyCode The code of the key that triggered the event.
     * @param keyState The state of the key (e.g., pressed, released, etc.).
     */
    virtual void keyEvent(UInt32 keyCode, UInt32 keyState);

LPRIVATE_IMP_UNIQUE(LView)
};

#endif // LVIEW_H
