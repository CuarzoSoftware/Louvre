#ifndef LVIEW_H
#define LVIEW_H

#include <LObject.h>
#include <LBitset.h>
#include <LRegion.h>
#include <LFramebuffer.h>
#include <LColor.h>
#include <GL/gl.h>
#include <thread>
#include <vector>
#include <list>
#include <map>

/**
 * @brief Base class for LScene views.
 *
 *  @anchor lview_detailed
 *
 * The LView class provides a base interface for creating views that can be shown in an LScene.\n
 * This class should be used for creating custom views that are different from those shipped with Louvre.\n
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
 * The outputs() virtual method must return a vector with the outputs where the view is currently visible. When the view changes, repaint() is called,
 * and all those intersected outputs are scheduled for repainting.
 * The scene informs which outputs a view is currently on based on its rect through the enteredOutput() and leftOutput() virtual methods.
 * So, those methods must be used to update the intersected outputs vector.
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
 * @note If you're using custom tools for rendering that can't handle opaque and translucent regions separately, set the view's translucent region to 
 *       `nullptr` (see the "Opaque and Translucent Regions" section). This way, only a single paintEvent() with blending enabled will be triggered.
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
 * To define which parts of the view can receive pointer or touch events, the inputRegion() must be implemented.\n
 * If `nullptr` is returned, the entire view is considered capable of receiving events.
 *
 * ## Input events
 *
 * Scenes can trigger specific input events on views through the pointerEnterEvent(), pointerMoveEvent(), keyEvent(), and other related methods.\n
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
    LView(UInt32 type, bool renderable, LView *parent) noexcept;

    LCLASS_NO_COPY(LView)

    /**
     * @brief Destructor for the LView class.
     */
    ~LView() noexcept;

    /// Types of views included with Louvre
    enum Type : UInt8
    {
        /// Undefined type
        UndefinedType,

        /// LLayerView
        LayerType,

        /// LSurfaceView
        SurfaceType,

        /// LTextureView
        TextureType,

        /// LSolidColorView
        SolidColorType,

        /// LSceneView
        SceneType
    };

    /**
     * @brief Toggles pointer events.
     *
     * Only views with this property enabled can appear on the LScene::pointerFocus() list after an LScene::handlePointerMoveEvent().
     *
     * Disabled by default in all view types except for LSurfaceView.
     *
     * @param enabled If `true`, the view will receive pointer events.
     */
    void enablePointerEvents(bool enabled) noexcept;

    /**
     * @brief Checks if pointer events are enabled.
     *
     * @returns `true` if the view receives pointer and touch events, `false` otherwise.
     */
    bool pointerEventsEnabled() const noexcept
    {
        return m_state.check(PointerEvents);
    }

    /**
     * @brief Checks if the view has pointer focus.
     *
     * Even if the scene pointer is over the input region it may return `false` if another view with block pointer enabled is in front.
     * 
     * @note Requires pointerEventsEnabled() to be `true`.
     *
     * @return `true` if the pointer/cursor is inside the input region; otherwise, `false`.
     */
    bool pointerIsOver() const noexcept
    {
        return m_state.check(PointerIsOver);
    }

    /**
     * @brief Toggles blocking pointer events to views behind the view's input region.
     *
     * @note Requires pointerEventsEnabled() to be `true`.
     * 
     * Enabled by default.
     *
     * @param enabled `true` to enable blocking; `false` to disable.
     */
    void enableBlockPointer(bool enabled) noexcept
    {
        m_state.setFlag(BlockPointer, enabled);
    }

    /**
     * @brief Checks if blocking of pointer events is enabled.
     *
     * @return `true` if blocking is enabled; otherwise, `false`.
     */
    bool blockPointerEnabled() const noexcept
    {
        return m_state.check(BlockPointer);
    }

    /**
     * @brief Toggles keyboard events.
     *
     * When enabled, the view will receive keyboard events and will be added to its parent LScene::keyboardFocus() vector.\n
     * Unlike pointer or touch events, keyboard events for views do not have focus semantics, this means that when enabled,
     * the view will always receive all keyboard events emitted by its parent scene.\n
     *
     * Disabled by default in all view types.
     *
     * @param enabled `true` to enable keyboard events, `false` to disable.
     */
    void enableKeyboardEvents(bool enabled) noexcept;

    /**
     * @brief Checks if keyboard events are enabled.
     *
     * @return `true` if keyboard events are enabled, `false` otherwise.
     *         Keyboard events are disabled by default.
     */
    bool keyboardEventsEnabled() const noexcept
    {
        return m_state.check(KeyboardEvents);
    }

    /**
     * @brief Toggles touch events.
     *
     * A view can be touched by multiple touch points simultaneously, see LScene::touchPoints() and LSceneTouchPoint::views()
     *
     * @note The inputRegion() property affects which part of the view can receive events.
     * 
     * Disabled by default in all view types except for LSurfaceView.
     *
     * @param enabled Set to `true` to enable touch events, `false` to disable them.
     */
    void enableTouchEvents(bool enabled) noexcept;

    /**
     * @brief Checks if touch events are enabled.
     *
     * @return `true` if touch events are enabled, `false` otherwise.
     */
    bool touchEventsEnabled() const noexcept
    {
        return m_state.check(TouchEvents);
    }

    /**
     * @brief Toggles blocking touch events to views behind the view's input region.
     *
     * @note Requires touchEventsEnabled() to be `true`.
     *
     * Enabled by default.
     *
     * @param enabled `true` to enable blocking; `false` to disable.
     */
    void enableBlockTouch(bool enabled) noexcept
    {
        m_state.setFlag(BlockTouch, enabled);
    }

    /**
     * @brief Checks if blocking of touch events to views behind the view's input region is enabled.
     *
     * @return `true` if blocking is enabled; otherwise, `false`.
     */
    bool blockTouchEnabled() const noexcept
    {
        return m_state.check(BlockTouch);
    }

    /**
     * @brief Forces a complete repaint of the view in the next rendering frame.
     */
    void damageAll() noexcept
    {
        markAsChangedOrder(false);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Gets the scene in which this view is currently embedded.
     *
     * @returns A pointer to the scene that contains this view, or `nullptr` if the view is not part of any scene.
     */
    LScene *scene() const noexcept
    {
        return m_scene;
    }

    /**
     * @brief Checks if the view is an LScene::mainView().
     */
    bool isLScene() const noexcept
    {
        return m_state.check(IsScene);
    }

    /**
     * @brief Gets the closest parent LSceneView.
     *
     * @returns A pointer to the closest LSceneView that contains this view, or `nullptr` if the view is not part of any LSceneView.
     */
    LSceneView *parentSceneView() const noexcept
    {
        if (parent())
        {
            if (parent()->type() == SceneType)
                return (LSceneView*)parent();

            return parent()->parentSceneView();
        }
        return nullptr;
    }

    /**
     * @brief Gets the identifier for the type of view.
     *
     * @see LView::Type.
     *
     * @returns The identifier representing the type of view.
     */
    UInt32 type() const noexcept { return m_type; }

    /**
     * @brief Checks if the view is renderable.
     *
     * This property indicates whether the view is capable of rendering content (check paintEvent()).\n
     * For example, all view types included in Louvre are renderable, except for LLayerView, which serves as a container for other views
     * but does not produce any output by itself.
     *
     * @return `true` if the view is renderable; otherwise, `false`.
     */
    bool isRenderable() const noexcept { return m_state.check(IsRenderable); };

    /**
     * @brief Schedules a repaint for all outputs where this view is currently visible.
     *
     * This method triggers a repaint for all outputs where this view is currently visible.\n
     * Outputs are those returned by the LView::outputs() method.
     */
    void repaint() const noexcept;

    /**
     * @brief Gets the parent of the view.
     *
     * @returns A pointer to the parent view, or `nullptr` if the view has no parent.
     */
    LView *parent() const noexcept { return m_parent; };

    /**
     * @brief Sets the new parent for the view and insert it at the end of its children list.
     *
     * This method sets the new parent for the view and inserts it at the end of its parent's children list.
     * If 'view' is set to `nullptr`, the parent is unset, and the view is unmapped.
     *
     * @param view The new parent view to be set.
     */
    void setParent(LView* view) noexcept;

    /**
     * @brief Inserts the view after the specified 'prev' view.
     * 
     * @note This changes the view's parent if `prev` has another parent view.
     *
     * @param prev The view after which this view will be inserted. If set to `nullptr`, the view will be inserted at the beginning of its current parent's children list.
     */
    void insertAfter(LView *prev) noexcept;

    /**
     * @brief Children views.
     *
     * @returns A reference to the list of child views.
     */
    const std::list<LView*> &children() const noexcept { return m_children; };

    /**
     * @brief Toggles the parent position offset.
     *
     * If enabled, the position returned by pos() will include the parent's position (parent()->pos()).
     *
     * Enabled by default.
     *
     * @param enabled If `true`, the parent's position will be applied to the view's position.
     */
    void enableParentOffset(bool enabled) noexcept
    {
        if (enabled == m_state.check(ParentOffset))
            return;

        m_state.setFlag(ParentOffset, enabled);

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Checks if pos() includes the parent's position.
     *
     * @returns `true` if the parent's offset is applied to the view position, `false` otherwise.
     */
    bool parentOffsetEnabled() const noexcept
    {
        return m_state.check(ParentOffset);
    }

    /**
     * @brief Gets the current position of the view with applied transformations.
     *
     * This method returns the current position of the view with any applied transformations.
     * 
     * @see nativePos().
     *
     * @returns The position of the view.
     */
    const LPoint& pos() const noexcept
    {
        m_tmpPoint = nativePos();

        if (parent())
        {
            if (parentScalingEnabled())
                m_tmpPoint *= parent()->scalingVector(parent()->type() == SceneType);

            if (parentOffsetEnabled())
                m_tmpPoint += parent()->pos();
        }

        return m_tmpPoint;
    }

    /**
     * @brief Gets the current size of the view with applied transformations.
     *
     * This method returns the current size of the view with any applied transformations.
     * 
     * @see nativeSize().
     *
     * @returns The size of the view in surface units.
     */
    const LSize &size() const noexcept
    {
        m_tmpSize = nativeSize();

        if (scalingEnabled())
            m_tmpSize *= scalingVector(true);

        if (parent() && parentScalingEnabled())
            m_tmpSize *= parent()->scalingVector(parent()->type() == SceneType);

        return m_tmpSize;
    }

    /**
     * @brief Toggles the use of the clippingRect() property.
     *
     * If enabled, the view will be clipped to the clippingRect() property. 
     *
     * Disabled by default.
     */
    void enableClipping(bool enabled) noexcept
    {
        if (m_state.check(Clipping) != enabled)
        {
            m_state.setFlag(Clipping, enabled);
            repaint();
        }
    }

    /**
     * @brief Checks if the view is currently being clipped to the clippingRect() property.
     *
     * This method returns `true` if the view is currently being clipped to the clippingRect() property.
     *
     * @returns `true` if the view is being clipped to clippingRect(), `false` otherwise.
     */
    bool clippingEnabled() const noexcept
    {
        return m_state.check(Clipping);
    }

    /**
     * @brief Sets a custom clipping rect.
     *
     * @see The clippingRect() property.
     * 
     * @note Requires clippingEnabled() to be `true`.
     *
     * @note The rect is not local to the view's position, if you need to clip the view without having to
     *       update the rect position each time the view moves, use parent clipping instead.
     *
     * @param rect The clipping rect in compositor-global coordinates.
     */
    void setClippingRect(const LRect &rect) noexcept
    {
        if (rect != m_clippingRect)
        {
            m_clippingRect = rect;

            if (!repaintCalled() && mapped())
                repaint();
        }
    }

    /**
     * @brief Gets the clipping rect set with setClippingRect().
     */
    const LRect &clippingRect() const noexcept
    {
        return m_clippingRect;
    }

    /**
     * @brief Toggles parent clipping.
     *
     * If enabled, the view will be clipped to the current parent view rect.
     *
     * Disabled by default.
     *
     * @param enabled If `true`, the view will be clipped to the current parent view rect.
     */
    void enableParentClipping(bool enabled) noexcept
    {
        if (enabled == m_state.check(ParentClipping))
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_state.setFlag(ParentClipping, enabled);
    }

    /**
     * @brief Checks if the view is clipped to the parent view rect.
     *
     * @see enableParentClipping().
     *
     * @returns `true` if the view is clipped to the current parent view rect, `false` otherwise.
     */
    bool parentClippingEnabled() const noexcept
    {
        return m_state.check(ParentClipping);
    }

    /**
     * @brief Toggles the use of the scalingVector().
     *
     * If enabled, the view's size will be multiplied by the scalingVector().
     *
     * Disabled by default.
     *
     * @param enabled If `true`, the view's size will be scaled.
     */
    void enableScaling(bool enabled) noexcept
    {
        if (enabled == m_state.check(Scaling))
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_state.setFlag(Scaling, enabled);
    }

    /**
     * @brief Checks if the scalingVector() is enabled.
     */
    bool scalingEnabled() const noexcept
    {
        return m_state.check(Scaling);
    }

    /**
     * @brief Sets the scalingVector() property.
     *
     * If scalingEnabled() is `true`, the view's size will be scaled using the provided scaling vector (nativeSize() * scalingVector()).
     * 
     * The default value is (1.f, 1.f).
     *
     * @warning Scaling should be used with moderation, preferably during animations, as damage tracking is disabled due to precision loss caused by scaling.
     *          This means the entire view is repainted if changes occur, which can be inefficient.
     */
    void setScalingVector(const LSizeF &scalingVector) noexcept
    {
        if (scalingVector == m_scalingVector)
            return;

        m_scalingVector = scalingVector;

        if (!repaintCalled() && mapped())
            repaint();
    }

    /**
     * @brief Gets the scaling vector.
     * 
     * @see setScalingVector().
     *
     * @param forceIgnoreParent If set to `false`, the vector is multiplied by the parent scaling vector.
     * @returns The scaling vector for the view's size.
     */
    const LSizeF &scalingVector(bool forceIgnoreParent = false) const noexcept
    {
        if (forceIgnoreParent)
            return m_scalingVector;

        m_tmpSizeF = m_scalingVector;

        if (parent() && parentScalingEnabled())
            m_tmpSizeF *= parent()->scalingVector(parent()->type() == SceneType);

        return m_tmpSizeF;
    }

    /**
     * @brief Toggles the scaling of size and position by the parent's scaling vector.
     *
     * If enabled, the view's size and position will be scaled by the parent's scalingVector().
     *
     * Disabled by default.
     *
     * @param enabled If `true`, the view's size and position will be scaled by the parent's scaling vector.
     */
    void enableParentScaling(bool enabled) noexcept
    {
        if (enabled == m_state.check(ParentScaling))
            return;

        if (!repaintCalled() && mapped())
            repaint();

        return m_state.setFlag(ParentScaling, enabled);
    }

    /**
     * @brief Checks if the size and position are scaled by the parent's scalingVector().
     *
     * Disabled by default.
     */
    bool parentScalingEnabled() const noexcept
    {
        return m_state.check(ParentScaling);
    }

    /**
     * @brief Toggles the view visibility.
     *
     * @note Enabling visibility does not guarantee that the view will be rendered, see mapped().
     * 
     * Enabled by default.
     */
    void setVisible(bool visible) noexcept
    {
        if (m_state.check(Visible) == visible)
            return;

        const bool prev { mapped() };
        m_state.setFlag(Visible, visible);

        if (!repaintCalled() && prev != mapped())
            repaint();
    }

    /**
     * @brief Checks if the view is marked as visible.
     *
     * This method indicates whether the view is marked as visible. 
     * However, it does not directly indicate if the view will be rendered.
     * To check if the view will be rendered, use the mapped() property instead.
     *
     * @returns `true` if the view is marked as visible, `false` otherwise.
     */
    bool visible() const noexcept
    {
        return m_state.check(Visible);
    }

    /**
     * @brief Checks if the view should be rendered, taking into consideration several conditions.
     *
     * This method indicates whether the view should be rendered, considering the nativeMapped() && visible() && parent() && parent()->mapped() condition.
     *
     * @returns `true` if the view should be rendered, `false` otherwise.
     */
    bool mapped() const noexcept
    {
        if (type() == SceneType && !parent())
            return visible();

        return visible() && nativeMapped() && parent() && parent()->mapped();
    }

    /**
     * @brief Set the view opacity.
     *
     * This method sets the view's opacity.
     * 
     * The default value is 1.f.
     *
     * @warning Opacity values != 1.f should be used with moderation, for example, only during animations, as the opaqueRegion() is not 
     *          ignored. This means that content behind the view is always repainted when the view changes.
     *
     * @param opacity The opacity value in the range [0.f, 1.f].
     */
    void setOpacity(Float32 opacity) noexcept
    {
        if (opacity < 0.f)
            opacity = 0.f;
        else if(opacity > 1.f)
            opacity = 1.f;

        if (opacity == m_opacity)
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_opacity = opacity;
    }

    /**
     * @brief Gets the view opacity.
     *
     * @param forceIgnoreParent If set to `false`, the result is multiplied by the parent's opacity.
     * @returns The view's opacity value in the range [0.0, 1.0].
     */
    Float32 opacity(bool forceIgnoreParent = false) const noexcept
    {
        if (forceIgnoreParent)
            return m_opacity;

        if (parentOpacityEnabled() && parent())
            return m_opacity * parent()->opacity(parent()->type() == SceneType);

        return m_opacity;
    }

    /**
     * @brief Toggles the use of the parent's opacity.
     *
     * If enabled, the view's opacity will be multiplied by its parent's opacity.
     *
     * Enabled by default.
     */
    void enableParentOpacity(bool enabled) noexcept
    {
        if (m_state.check(ParentOpacity) == enabled)
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_state.setFlag(ParentOpacity, enabled);
    }

    /**
     * @brief Checks if the view's opacity is multiplied by its parent's opacity.
     */
    bool parentOpacityEnabled() const noexcept
    {
        return m_state.check(ParentOpacity);
    }

    /**
     * @brief Toggles forcing triggering the requestNextFrame() event.
     *
     * When enabled, requestNextFrame() will be called even if the view
     * is occluded or not mapped.
     * 
     * Disabled by default.
     */
    void enableForceRequestNextFrame(bool enabled) noexcept
    {
        m_state.setFlag(ForceRequestNextFrame, enabled);
    }

    /**
     * @brief Checks if forcing requestNextFrame() is enabled.
     */
    bool forceRequestNextFrameEnabled() const noexcept
    {
        return m_state.check(ForceRequestNextFrame);
    }

    /**
     * @brief Sets a custom alpha/color blending function for the view.
     *
     * Sets the OpenGL blend function for the view. Refer to the documentation
     * of [glBlendFuncSeparate()](https://docs.gl/es2/glBlendFuncSeparate) for more information.
     *
     * @note Requires autoBlendFuncEnabled() to be disabled.
     */
    void setBlendFunc(const LBlendFunc &blendFunc) noexcept
    {
        if (blendFunc.sRGBFactor != m_blendFunc.sRGBFactor || blendFunc.dRGBFactor != m_blendFunc.dRGBFactor ||
            blendFunc.sAlphaFactor != m_blendFunc.sAlphaFactor || blendFunc.dAlphaFactor != m_blendFunc.dAlphaFactor)
        {
            m_blendFunc = blendFunc;
            repaint();
        }
    }

    /**
     * @brief Gets the blending function set with setBlendFunc().
     * 
     * @see enableAutoBlendFunc()
     */
    const LBlendFunc &blendFunc() const noexcept
    {
        return m_blendFunc;
    }

    /**
     * @brief Toggles the automatic blend function adjustment.
     *
     * When enabled, the blend function dynamically adjusts based on whether the rendered
     * content is premultiplied alpha or not.
     *
     * @note When enabled, the blend function set with setBlendFunc() is ignored.
     * 
     * Enabled by default.
     */
    void enableAutoBlendFunc(bool enabled) noexcept
    {
        if (enabled == m_state.check(AutoBlendFunc))
            return;

        if (!repaintCalled() && mapped())
            repaint();

        m_state.setFlag(AutoBlendFunc, enabled);
    }

    /**
     * @brief Checks whether the automatic blend function adjustment is enabled.
     */
    bool autoBlendFuncEnabled() const noexcept
    {
        return m_state.check(AutoBlendFunc);
    }

    /**
     * @brief Set the color factor.
     *
     * Multiplies the view's RGBA color components by the given values.\n
     * Set to (1.0, 1.0, 1.0, 1.0) by default, which has no effect on the colors.
     */
    void setColorFactor(const LRGBAF &colorFactor) noexcept
    {
        if (m_colorFactor.r != colorFactor.r ||
            m_colorFactor.g != colorFactor.g ||
            m_colorFactor.b != colorFactor.b ||
            m_colorFactor.a != colorFactor.a)
        {
            m_colorFactor = colorFactor;
            repaint();
            m_state.setFlag(ColorFactor, m_colorFactor.r != 1.f || m_colorFactor.g != 1.f || m_colorFactor.b != 1.f || m_colorFactor.a != 1.f);
        }
    }

    /**
     * @brief Gets the color factor.
     *
     * See setColorFactor().
     */
    const LRGBAF &colorFactor() const noexcept
    {
        return m_colorFactor;
    }

    /**
     * @brief Gets the bounding box of the view and all its mapped children.
     *
     * This method returns a box containing the view and all its mapped children, even if the children
     * are outside or clipped by the view's rect.
     *
     * @return The bounding box of the view and its mapped children.
     */
    LBox boundingBox() const noexcept
    {
        const LPoint &p { pos() };
        const LPoint &s { size() };

        LBox box
        {
            p.x(),
            p.y(),
            p.x() + s.w(),
            p.y() + s.h(),
        };

        LBox childBox;

        for (LView *child : children())
        {
            if (!child->mapped())
                continue;

            childBox = child->boundingBox();

            if (childBox.x1 < box.x1)
                box.x1 = childBox.x1;

            if (childBox.y1 < box.y1)
                box.y1 = childBox.y1;

            if (childBox.x2 > box.x2)
                box.x2 = childBox.x2;

            if (childBox.y2 > box.y2)
                box.y2 = childBox.y2;
        }

        return box;
    }

    /**
     * @brief Tells whether the view should be rendered.
     *
     * @return `true` if the view should be rendered without considering visible(), otherwise `false`.
     */
    virtual bool nativeMapped() const noexcept = 0;

    /**
     * @brief Gets the position of the view without any transformations applied.
     *
     * Must return the position of the view in compositor-global coordinates.
     *
     * @return The position of the view as an LPoint object.
     */
    virtual const LPoint &nativePos() const noexcept = 0;

    /**
     * @brief Gets the size of the view without any transformations applied.
     *
     * Must return the size of the view in surface coordinates.
     *
     * @return The size of the view as an LSize object.
     */
    virtual const LSize &nativeSize() const noexcept = 0;

    /**
     * @brief Gets the scale of the view buffer content.
     *
     * This property is primarily used by views that contain a buffer like for example the LSceneView, LSurfaceView and LTextureView types.
     *
     * @return The buffer scale as an Int32 value.
     */
    virtual Float32 bufferScale() const noexcept = 0;

    /**
     * @brief Notifies that portion of the view is visible on the given output.
     *
     * This method is invoked by a LScene when the view's rect intersects an output.
     *
     * @param output The LOutput where the view is visible.
     */
    virtual void enteredOutput(LOutput *output) noexcept = 0;

    /**
     * @brief Notifies that the view is no longer visible on the given output.
     *
     * This method is invoked by a LScene when the view's rect no longer intersects an output.
     *
     * @param output The LOutput from which the view is no longer visible.
     */
    virtual void leftOutput(LOutput *output) noexcept = 0;

    /**
     * @brief Gets a vector of output pointers on which the view is currently visible.
     *
     * Must return a vector of output pointers where the view is currently visible.\n
     * Use the enteredOutput() and leftOutput() methods to update the vector.
     *
     * @return A reference to a vector of LOutput pointers representing the outputs where the view is visible.
     */
    virtual const std::vector<LOutput*> &outputs() const noexcept = 0;

    /**
     * @brief Notifies that the view has been rendered on the given output.
     *
     * This method is called by the closest parent scene view and should be used to clear the previous view damage or update its content.\n
     * If forceRequestNextFrameEnabled() is `true`, this method is always called.
     *
     * @param output The LOutput on which the view is rendered.
     */
    virtual void requestNextFrame(LOutput *output) noexcept = 0;

    /**
     * @brief Gets the region within the view rect that needs to be repainted.
     *
     * The region rects are specified in surface coordinates within the view,
     * without any scaling, clipping, or offset transformations applied.
     * 
     * It should be cleared after requestNextFrame() is called.
     * 
     * If `nullptr` is returned, the entire view rect will be considered damaged.
     * 
     * If the view has no damage, simply pass an empty LRegion (not `nullptr`).
     *
     * @return A pointer to the LRegion specifying the damaged area within the view,
     *         or `nullptr` if the entire view rect is damaged.
     */
    virtual const LRegion *damage() const noexcept = 0;

    /**
     * @brief Gets the current translucent region.
     * 
     * The region must be specified in surface-local coordinates, without any scaling, clipping, or offset transformations applied.
     * 
     * If `nullptr` is returned, the entire view rect will be considered translucent.
     */
    virtual const LRegion *translucentRegion() const noexcept = 0;

    /**
     * @brief Gets the current opaque region.
     * 
     * The region must be specified in surface-local coordinates without any scaling, clipping, or offset transformations applied.
     * 
     * If `nullptr` is returned, the inverse of the translucent region will be considered opaque.
     * 
     * @note You could only specify the translucent region.
     */
    virtual const LRegion *opaqueRegion() const noexcept = 0;

    /**
     * @brief Gets the current input region.
     * 
     * Region within the view rect that can receive pointer and touch events.
     * 
     * The region must be specified in surface-local coordinates without any scaling, clipping, or offset transformations applied.
     * 
     * If `nullptr` is returned, the entire view will receive input.
     */
    virtual const LRegion *inputRegion() const noexcept = 0;

    /**
     * @brief Requests to paint a region of the view to the current framebuffer.
     *
     * This method is used by the closest parent LSceneView to request the view to paint a specified region
     * on the current framebuffer. Painting can be performed using the provided LPainter object.
     *
     * @note Using LPainter is optional.
     */
    virtual void paintEvent(const PaintEventParams &params) noexcept = 0;

    /**
     * @brief Handles a pointer enter event within the view.
     *
     * This event is only triggered if pointerEventsEnabled() is set to `true`.
     */
    virtual void pointerEnterEvent(const LPointerEnterEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer move event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerMoveEvent(const LPointerMoveEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer leave event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerLeaveEvent(const LPointerLeaveEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer button event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerButtonEvent(const LPointerButtonEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer scroll event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerScrollEvent(const LPointerScrollEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer swipe begin event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer swipe update event within the view.
     *
     * This event is only triggered if pointerSwipeBeginEvent() was called before.
     */
    virtual void pointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer swipe end event within the view.
     *
     * This event is only triggered if pointerSwipeBeginEvent() was called before.
     */
    virtual void pointerSwipeEndEvent(const LPointerSwipeEndEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer pinch begin event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerPinchBeginEvent(const LPointerPinchBeginEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer pinch update event within the view.
     *
     * This event is only triggered if pointerPinchBeginEvent() was called before.
     */
    virtual void pointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer pinch end event within the view.
     *
     * This event is only triggered if pointerPinchBeginEvent() was called before.
     */
    virtual void pointerPinchEndEvent(const LPointerPinchEndEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer hold begin event within the view.
     *
     * This event is only triggered if pointerEnterEvent() was called before, and therefore when hasPointerFocus() returns `true`.
     */
    virtual void pointerHoldBeginEvent(const LPointerHoldBeginEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a pointer hold end event within the view.
     *
     * This event is only triggered if pointerHoldBeginEvent() was called before.
     */
    virtual void pointerHoldEndEvent(const LPointerHoldEndEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle the key event within the view.
     *
     * Keyboard events are triggered only if keyboardEventsEnabled() is set to `true`.
     */
    virtual void keyEvent(const LKeyboardKeyEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a touch down event within the view.
     *
     * Touch events are triggered only if touchEventsEnabled() is set to `true`.
     */
    virtual void touchDownEvent(const LTouchDownEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a touch move event within the view.
     *
     * This event is only triggered if a touchDownEvent() was emitted before.
     */
    virtual void touchMoveEvent(const LTouchMoveEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a touch up event within the view.
     *
     * This event is only triggered if a touchDownEvent() was emitted before.
     */
    virtual void touchUpEvent(const LTouchUpEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a touch frame event within the view.
     *
     * This event is only triggered if a touchDownEvent() was emitted before.
     */
    virtual void touchFrameEvent(const LTouchFrameEvent &event) { L_UNUSED(event) };

    /**
     * @brief Handle a touch cancel event within the view.
     *
     * This event is only triggered if a touchDownEvent() was emitted before.
     */
    virtual void touchCancelEvent(const LTouchCancelEvent &event) { L_UNUSED(event) };

    enum LViewState : UInt64
    {
        // LView
        Destroyed               = static_cast<UInt64>(1) << 0,
        IsScene                 = static_cast<UInt64>(1) << 1,
        IsRenderable            = static_cast<UInt64>(1) << 2,

        PointerEvents           = static_cast<UInt64>(1) << 3,
        KeyboardEvents          = static_cast<UInt64>(1) << 4,
        TouchEvents             = static_cast<UInt64>(1) << 5,

        BlockPointer            = static_cast<UInt64>(1) << 6,
        BlockTouch              = static_cast<UInt64>(1) << 7,

        RepaintCalled           = static_cast<UInt64>(1) << 8,
        ColorFactor             = static_cast<UInt64>(1) << 9,
        Visible                 = static_cast<UInt64>(1) << 10,
        Scaling                 = static_cast<UInt64>(1) << 11,
        ParentScaling           = static_cast<UInt64>(1) << 12,
        ParentOffset            = static_cast<UInt64>(1) << 13,
        Clipping                = static_cast<UInt64>(1) << 14,
        ParentClipping          = static_cast<UInt64>(1) << 15,
        ParentOpacity           = static_cast<UInt64>(1) << 16,
        ForceRequestNextFrame   = static_cast<UInt64>(1) << 17,
        AutoBlendFunc           = static_cast<UInt64>(1) << 18,

        PointerIsOver           = static_cast<UInt64>(1) << 19,

        PendingSwipeEnd         = static_cast<UInt64>(1) << 20,
        PendingPinchEnd         = static_cast<UInt64>(1) << 21,
        PendingHoldEnd          = static_cast<UInt64>(1) << 22,

        PointerMoveDone         = static_cast<UInt64>(1) << 23,
        PointerButtonDone       = static_cast<UInt64>(1) << 24,
        PointerScrollDone       = static_cast<UInt64>(1) << 25,
        PointerSwipeBeginDone   = static_cast<UInt64>(1) << 26,
        PointerSwipeUpdateDone  = static_cast<UInt64>(1) << 27,
        PointerSwipeEndDone     = static_cast<UInt64>(1) << 28,
        PointerPinchBeginDone   = static_cast<UInt64>(1) << 29,
        PointerPinchUpdateDone  = static_cast<UInt64>(1) << 30,
        PointerPinchEndDone     = static_cast<UInt64>(1) << 31,
        PointerHoldBeginDone    = static_cast<UInt64>(1) << 32,
        PointerHoldEndDone      = static_cast<UInt64>(1) << 33,
        KeyDone                 = static_cast<UInt64>(1) << 34,
        TouchDownDone           = static_cast<UInt64>(1) << 35,
        TouchMoveDone           = static_cast<UInt64>(1) << 36,
        TouchUpDone             = static_cast<UInt64>(1) << 37,
        TouchFrameDone          = static_cast<UInt64>(1) << 38,
        TouchCancelDone         = static_cast<UInt64>(1) << 39,

        // LTextureView
        CustomColor             = static_cast<UInt64>(1) << 40,
        CustomDstSize           = static_cast<UInt64>(1) << 41,
        CustomSrcRect           = static_cast<UInt64>(1) << 42,

        // LSurfaceView
        Primary                 = static_cast<UInt64>(1) << 43,
        CustomPos               = static_cast<UInt64>(1) << 44,
        CustomInputRegion       = static_cast<UInt64>(1) << 45,
        CustomTranslucentRegion = static_cast<UInt64>(1) << 46,
        AlwaysMapped            = static_cast<UInt64>(1) << 47,
    };

    // This is used for detecting changes on a view since the last time it was drawn on a specific output
    struct ViewThreadData
    {
        LRegion prevClipping;
        LRGBAF prevColorFactor;
        LRect prevRect;
        LRect prevLocalRect;
        LOutput *o { nullptr };
        Float32 prevOpacity { 1.f };
        UInt32 lastRenderedDamageId { 0 };
        bool prevColorFactorEnabled { false };
        bool changedOrder { true };
        bool prevMapped { false };
    };

    // This is used to prevent invoking heavy methods
    struct ViewCache
    {
        ViewThreadData *voD;
        LRect rect;
        LRect localRect;
        LRegion damage;
        LRegion translucent;
        LRegion opaque;
        LRegion opaqueOverlay;
        Float32 opacity;
        LSizeF scalingVector;
        bool mapped { false };
        bool occluded { false };
        bool scalingEnabled;
    };

protected:
    friend class LScene;
    friend class LSceneView;
    friend class LCompositor;
    mutable LBitset<LViewState> m_state { Visible | ParentOffset | ParentOpacity | BlockPointer | AutoBlendFunc };
    LScene *m_scene { nullptr };
    LView *m_parent { nullptr };
    std::list<LView*> m_children;
    std::list<LView*>::iterator m_parentLink;
    UInt32 m_type;
    Float32 m_opacity { 1.f };
    LSizeF m_scalingVector { 1.f, 1.f };
    LRect m_clippingRect;
    LBlendFunc m_blendFunc {
                           GL_SRC_ALPHA,
                           GL_ONE_MINUS_SRC_ALPHA,
                           GL_SRC_ALPHA,
                           GL_ONE_MINUS_SRC_ALPHA };
    LRGBAF m_colorFactor {1.f, 1.f, 1.f, 1.f};
    mutable LPoint m_tmpPoint;
    mutable LSize m_tmpSize;
    mutable LSizeF m_tmpSizeF;
    ViewCache m_cache;
    std::map<std::thread::id,ViewThreadData> m_threadsMap;

    bool repaintCalled() const noexcept
    {
        return m_state.check(RepaintCalled);
    }

    static void removeFlagWithChildren(LView *view, UInt64 flag)
    {
        view->m_state.remove(flag);

        for (LView *child : view->children())
            removeFlagWithChildren(child, flag);
    }

    void removeThread(std::thread::id thread);
    void markAsChangedOrder(bool includeChildren = true);
    void damageScene(LSceneView *scene, bool includeChildren);
    void sceneChanged(LScene *newScene);
};

#endif // LVIEW_H
