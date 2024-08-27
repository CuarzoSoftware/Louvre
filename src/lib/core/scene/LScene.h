#ifndef LSCENE_H
#define LSCENE_H

#include <LView.h>
#include <LSeat.h>
#include <LObject.h>
#include <LPointer.h>
#include <LKeyboard.h>

/**
 * @brief Scene
 *
 * @anchor lscene_detailed
 *
 * The LScene class is an optional utility that significantly simplifies rendering.\n
 * It encompasses a primary LSceneView which can host multiple children and even nested scenes.\n
 * A single LScene can drive multiple outputs and also manage Wayland input events, while also providing per-view input events.\n
 * You might opt to leverage LScene for rendering instead of relying solely on the basic rendering functions from the LPainter class or your custom OpenGL shaders.
 *
 * LScene achieves high efficiency by rendering only damaged regions and avoiding rendering content obscured by opaque areas.
 *
 * Alternatively, you can still use your own OpenGL shaders or LPainter functions for rendering, either in conjunction with LScene or independently.\n
 * These approaches can be applied before or after handlePaintGL() is called, or by creating custom LViews that override the LView::paintEvent() virtual method.
 *
 * ### Rendering
 *
 * For proper rendering with LScene, you need to "plug" the following methods into each LOutput you intend to manage:
 *
 * - handleInitializeGL()   -> LOutput::initializeGL()
 * - handlePaintGL()        -> LOutput::paintGL()
 * - handleMoveGL()         -> LOutput::moveGL()
 * - handleResizeGL()       -> LOutput::resizeGL()
 * - handleUninitializeGL() -> LOutput::uninitializeGL()
 *
 * @warning Make sure to "plug" the scene to all the output events mentioned earlier.
 *          Failing to do so may result in scene initialization issues and memory leaks.
 *
 * For example, like this:
 *
 * @code

#include <LOutput.h>

class YourCustomOutput : public LOutput
{
    ...

    void initializeGL(LOutput *output) override
    {
        scene->handleInitializeGL(output);
    }

    void paintGL(LOutput *output) override
    {
        scene->handlePaintGL(output);
    }

    void moveGL(LOutput *output) override
    {
        scene->handleMoveGL(output);
    }

    void resizeGL(LOutput *output) override
    {
        scene->handleResizeGL(output);
    }

    void uninitializeGL(LOutput *output) override
    {
        scene->handleUninitializeGL(output);
    }
};

 * @endcode
 *
 * ### Input Events
 *
 * Similar to rendering, you can integrate LScene into LPointer, LKeyboard and LTouch for managing input events.
 *
 * @see EventOptions
 */
class Louvre::LScene : public LObject
{
public:

    /**
     * @brief Custom options for input event handlers
     */
    enum EventOptions : UInt8
    {
        /// All options disabled, input events will only be dispatched to views acordingly.
        OptionsDisabled     = static_cast<UInt8>(0),

        /// Input events will be dispatched to client surfaces acordingly.
        WaylandEvents       = static_cast<UInt8>(1) << 0,

        /// The scene will constraint the cursor within surfaces when allowed clients require it.
        PointerConstraints  = static_cast<UInt8>(1) << 1,

        /// Auxiliary functionality of the default Louvre implementation, such as exiting the compositor when Ctrl + Shift + Esc is pressed, will be handled.
        AuxFunc             = static_cast<UInt8>(1) << 2
    };

    /**
     * @brief Input filter flags used by viewAt().
     *
     * @see viewAt()
     */
    enum InputFilter : UInt8
    {
        /// All views will be considered.
        FilterDisabled  = static_cast<UInt8>(0),

        /// Views with pointer events enabled will be considered.
        Pointer         = static_cast<UInt8>(1) << 0,

        /// Views with keyboard events enabled will be considered.
        Keyboard        = static_cast<UInt8>(1) << 1,

        /// Views with touch events enabled will be considered.
        Touch           = static_cast<UInt8>(1) << 2,
    };

    /**
     * @brief Constructor for LScene.
     */
    LScene();

    LCLASS_NO_COPY(LScene)

    /**
     * @brief Destructor for LScene.
     */
    ~LScene();

    /**
     * @brief Enables or disables automatic repainting of child views.
     *
     * When disabled, repaint() requests from child views will be ignored.
     * This can be useful when modifying views during an `LOutput::paintGL()` event
     * to prevent the output from immediately scheduling a new repaint.
     *
     * Enabled by default.
     *
     * @param enabled If true, automatic repainting is enabled; if false, it is disabled.
     */
    void enableAutoRepaint(bool enabled) noexcept;

    /**
     * @brief Checks if automatic repainting of child views is currently enabled.
     *
     * @see enableAutoRepaint
     */
    bool autoRepaintEnabled() const noexcept;

    /**
     * @brief Vector of views with pointer focus.
     *
     * This vector contains views that have pointer events enabled and whose input region intersects
     * with the current pointer position.
     *
     * The vector is ordered with the topmost views first and the bottommost views last.\n
     * Views with the property LView::blockInputEnabled() disabled allow views behind them
     * to also receive pointer events, which is why multiple views can be in focus at the same time.
     */
    const std::vector<LView*> &pointerFocus() const;

    /**
     * @brief Vector of views with keyboard focus.
     *
     * Views with keyboard focus are simply those whose LView::keyboardEventsEnabled() property is enabled.
     */
    const std::vector<LView*> &keyboardFocus() const;

    /**
     * @brief Vector of active touch points managed within the scene.
     */
    const std::vector<LSceneTouchPoint*> &touchPoints() const;

    /**
     * @brief Searches for a touch point within the scene by its ID.
     *
     * @param id The ID of the touch point to search for.
     * @return A pointer to the touch point if found, `nullptr` otherwise.
     */
    LSceneTouchPoint *findTouchPoint(Int32 id) const;

    /**
     * @brief Handle the OpenGL initialization for an LOutput.
     *
     * This method should be integrated into LOutput::initializeGL() to properly handle OpenGL initialization for the associated output.
     *
     * @param output The LOutput instance to handle OpenGL initialization for.
     */
    void handleInitializeGL(LOutput *output);

    /**
     * @brief Handle the OpenGL painting for an LOutput.
     *
     * This method should be integrated into LOutput::paintGL() to properly handle OpenGL painting for the associated output.
     *
     * @param output The LOutput instance to handle OpenGL painting for.
     */
    void handlePaintGL(LOutput *output);

    /**
     * @brief Handle the OpenGL movement for an LOutput.
     *
     * This method should be integrated into LOutput::moveGL() to properly handle OpenGL movement for the associated output.
     *
     * @param output The LOutput instance to handle OpenGL movement for.
     */
    void handleMoveGL(LOutput *output);

    /**
     * @brief Handle the OpenGL resizing for an LOutput.
     *
     * This method should be integrated into LOutput::resizeGL() to properly handle OpenGL resizing for the associated output.
     *
     * @param output The LOutput instance to handle OpenGL resizing for.
     */
    void handleResizeGL(LOutput *output);

    /**
     * @brief Handle the OpenGL uninitialization for an LOutput.
     *
     * This method should be integrated into LOutput::uninitializeGL() to properly handle OpenGL uninitialization for the associated output.
     *
     * @param output The LOutput instance to handle OpenGL uninitialization for.
     */
    void handleUninitializeGL(LOutput *output);

    /**
     * @brief Handle pointer movement event.
     *
     * This method should be integrated into LPointer::pointerMoveEvent() to effectively manage pointer movement events.
     *
     * @param event The pointer move event to handle.
     * @param options Custom event options. @see EventOptions.
     */
    void handlePointerMoveEvent(const LPointerMoveEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer button event.
     *
     * This method should be integrated into LPointer::pointerButtonEvent() to handle pointer button events.
     *
     * @param event The pointer button event to handle.
     */
    void handlePointerButtonEvent(const LPointerButtonEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer scroll event.
     *
     * This method should be integrated into LPointer::pointerScrollEvent() to manage pointer scroll events.
     *
     * @param event The pointer scroll event to handle.
     */
    void handlePointerScrollEvent(const LPointerScrollEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe begin event.
     *
     * This method should be integrated into LPointer::pointerSwipeBeginEvent() to manage pointer swipe begin events.
     *
     * @param event The pointer swipe begin event to handle.
     */
    void handlePointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe update event.
     *
     * This method should be integrated into LPointer::pointerSwipeUpdateEvent() to manage pointer swipe update events.
     *
     * @param event The pointer swipe update event to handle.
     */
    void handlePointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe end event.
     *
     * This method should be integrated into LPointer::pointerSwipeEndEvent() to manage pointer swipe end events.
     *
     * @param event The pointer swipe end event to handle.
     */
    void handlePointerSwipeEndEvent(const LPointerSwipeEndEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch begin event.
     *
     * This method should be integrated into LPointer::pointerPinchBeginEvent() to manage pointer pinch begin events.
     *
     * @param event The pointer pinch begin event to handle.
     */
    void handlePointerPinchBeginEvent(const LPointerPinchBeginEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch update event.
     *
     * This method should be integrated into LPointer::pointerPinchUpdateEvent() to manage pointer pinch update events.
     *
     * @param event The pointer pinch update event to handle.
     */
    void handlePointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch end event.
     *
     * This method should be integrated into LPointer::pointerPinchEndEvent() to manage pointer pinch end events.
     *
     * @param event The pointer pinch end event to handle.
     */
    void handlePointerPinchEndEvent(const LPointerPinchEndEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer hold begin event.
     *
     * This method should be integrated into LPointer::pointerHoldBeginEvent() to manage pointer hold begin events.
     *
     * @param event The pointer hold begin event to handle.
     */
    void handlePointerHoldBeginEvent(const LPointerHoldBeginEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer hold end event.
     *
     * This method should be integrated into LPointer::pointerHoldEndEvent() to manage pointer hold end events.
     *
     * @param event The pointer hold end event to handle.
     */
    void handlePointerHoldEndEvent(const LPointerHoldEndEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle keyboard key event.
     *
     * This method should be integrated into LKeyboard::keyEvent() to handle key events.
     */
    void handleKeyboardKeyEvent(const LKeyboardKeyEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch down event.
     *
     * This method should be integrated into LTouch::touchDownEvent() to manage touch down events.
     *
     * @param event The touch down event to handle.
     * @param globalPos The event position transformed to global coordinates.
     */
    void handleTouchDownEvent(const LTouchDownEvent &event, const LPointF &globalPos, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch move event.
     *
     * This method should be integrated into LTouch::touchMoveEvent() to manage touch move events.
     *
     * @param event The touch move event to handle.
     * @param globalPos The event position transformed to global coordinates.
     */
    void handleTouchMoveEvent(const LTouchMoveEvent &event, const LPointF &globalPos, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch up event.
     *
     * This method should be integrated into LTouch::touchUpEvent() to manage touch up events.
     *
     * @param event The touch up event to handle.
     */
    void handleTouchUpEvent(const LTouchUpEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch frame event.
     *
     * This method should be integrated into LTouch::touchFrameEvent() to manage touch frame events.
     *
     * @param event The touch frame event to handle.
     */
    void handleTouchFrameEvent(const LTouchFrameEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch cancel event.
     *
     * This method should be integrated into LTouch::touchCancelEvent() to manage touch cancel events.
     *
     * @param event The touch cancel event to handle.
     */
    void handleTouchCancelEvent(const LTouchCancelEvent &event, LBitset<EventOptions> options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Retrieve the main (root) view of the scene.
     *
     * This method returns the main LSceneView associated with the LScene.
     *
     * @return A pointer to the main LSceneView of the scene.
     */
    LSceneView *mainView() const;

    /**
     * @brief Retrieve the view located at the specified position.
     *
     * This method returns the first LView whose input region intersects the given position.
     *
     * @param pos The position to query.
     * @param type The type of view to search for. Passing LView::Type::Undefined disables the type filter.
     * @param filter Additional flags for searching only views with pointer, keyboard and/or touch events enabled.
     *
     * @return A pointer to the LView at the specified position, or `nullptr` if no view is found.
     */
    LView *viewAt(const LPoint &pos, LView::Type type = LView::UndefinedType, LBitset<InputFilter> filter = FilterDisabled);

LPRIVATE_IMP_UNIQUE(LScene)
};

#endif // LSCENE_H
