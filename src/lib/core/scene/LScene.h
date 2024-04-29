#ifndef LSCENE_H
#define LSCENE_H

#include <LView.h>
#include <LSeat.h>
#include <LObject.h>
#include <LPointer.h>
#include <LKeyboard.h>

/**
 * The LScene class is an optional utility that significantly simplifies rendering.
 * It encompasses a primary LSceneView which can host multiple children and even nested scenes.
 * A single LScene can drive multiple outputs and also manage pointer and keyboard Wayland events, while also providing per-view input events.
 * You might opt to leverage LScene for rendering instead of relying solely on the basic rendering functions from the LPainter class or your custom OpenGL shaders.
 *
 * LScene achieves high efficiency by rendering only damaged regions and avoiding rendering content obscured by opaque areas.
 *
 * Alternatively, you can still use your own OpenGL shaders or LPainter functions for rendering, either in conjunction with LScene or independently.
 * These approaches can be applied before or after handlePaintGL() is invoked, or by creating your custom LView that overrides the LView::paintRect() virtual method.
 *
 * ### Rendering
 *
 * For proper rendering with LScene, you need to "plug" the following methods into each LOutput you intend to manage with LScene:
 *
 * - handleInitializeGL()   -> LOutput::initializeGL()
 * - handlePaintGL()        -> LOutput::paintGL()
 * - handleMoveGL()         -> LOutput::moveGL()
 * - handleResizeGL()       -> LOutput::resizeGL()
 * - handleUninitializeGL() -> LOutput::uninitializeGL()
 *
 * @warning Make sure to "plug" the scene to all the output events mentioned earlier.
 *          Failing to do so may result in scene initialization issues, memory leaks, and potential compositor crashes.
 *
 * For example, like this:
 *
 * @code

#include <LOutput.h>

class YourCustomOutput : public LOutput
{
    LScene *scene;

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
 * ### Pointer Events
 *
 * Similar to rendering, you can integrate the handlePointerXXX methods into the LPointer class.\n
 * This enables LScene to dispatch pointer events to each LView, invoking methods such as LView::pointerEnterEvent(), LView::pointerMoveEvent(), and more.
 * Additionally, the scene can manage pointer focus for LSurfaces and transmit pointer events accordingly to clients.
 * You also have the option to disable this behavior if you prefer to handle this logic independently.
 * To deactivate the handling of Wayland events, use the enableHandleWaylandPointerEvents() method.
 *
 * ### Keyboard Events
 *
 * Similar to rendering, you can integrate the handleKeyXXX methods into the LKeyboard class.\n
 * This empowers LScene to dispatch keyboard events to each LView, triggering methods like LView::keyEvent(), LView::keyModifiersEvent(), and more.
 * Furthermore, the scene can manage keyboard focus for LSurfaces and transmit keyboard events correspondingly to clients.
 * If desired, you can also opt to disable this behavior and manage the logic independently.
 * To deactivate the handling of Wayland events, utilize the enableHandleWaylandKeyboardEvents() method.
 */
class Louvre::LScene : public LObject
{
public:

    using EventOptionsFlags = UInt8;

    enum EventOptions : EventOptionsFlags
    {
        Disabled            = static_cast<UInt8>(0),
        WaylandEvents       = static_cast<UInt8>(1) << 0,
        PointerConstraints  = static_cast<UInt8>(1) << 1,
        AuxFunc             = static_cast<UInt8>(1) << 2
    };

    enum InputFilter : UInt8
    {
        Pointer     = static_cast<UInt8>(1) << 0,
        Keyboard    = static_cast<UInt8>(1) << 1,
        Touch       = static_cast<UInt8>(1) << 2,
    };

    /**
     * @brief Default constructor for LScene.
     */
    LScene();

    LCLASS_NO_COPY(LScene)

    /**
     * @brief Destructor for LScene.
     */
    ~LScene();

    // TODO ADD DOC
    const std::vector<LView*> &pointerFocus() const;
    const std::vector<LView*> &keyboardFocus() const;
    const std::vector<LSceneTouchPoint*> &touchPoints() const;
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
     * @param outLocalPos Stores the local position of the first view found under the cursor. Pass `nullptr` if not needed.
     * @return The first LView found under the cursor.
     */
    void handlePointerMoveEvent(const LPointerMoveEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer button event.
     *
     * This method should be integrated into LPointer::pointerButtonEvent() to handle pointer button events.
     *
     * @param event The pointer button event to handle.
     */
    void handlePointerButtonEvent(const LPointerButtonEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer scroll event.
     *
     * This method should be integrated into LPointer::pointerScrollEvent() to manage pointer scroll events.
     *
     * @param event The pointer scroll event to handle.
     */
    void handlePointerScrollEvent(const LPointerScrollEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe begin event.
     *
     * This method should be integrated into LPointer::pointerSwipeBeginEvent() to manage pointer swipe begin events.
     *
     * @param event The pointer swipe begin event to handle.
     */
    void handlePointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe update event.
     *
     * This method should be integrated into LPointer::pointerSwipeUpdateEvent() to manage pointer swipe update events.
     *
     * @param event The pointer swipe update event to handle.
     */
    void handlePointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer swipe end event.
     *
     * This method should be integrated into LPointer::pointerSwipeEndEvent() to manage pointer swipe end events.
     *
     * @param event The pointer swipe end event to handle.
     */
    void handlePointerSwipeEndEvent(const LPointerSwipeEndEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch begin event.
     *
     * This method should be integrated into LPointer::pointerPinchBeginEvent() to manage pointer pinch begin events.
     *
     * @param event The pointer pinch begin event to handle.
     */
    void handlePointerPinchBeginEvent(const LPointerPinchBeginEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch update event.
     *
     * This method should be integrated into LPointer::pointerPinchUpdateEvent() to manage pointer pinch update events.
     *
     * @param event The pointer pinch update event to handle.
     */
    void handlePointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer pinch end event.
     *
     * This method should be integrated into LPointer::pointerPinchEndEvent() to manage pointer pinch end events.
     *
     * @param event The pointer pinch end event to handle.
     */
    void handlePointerPinchEndEvent(const LPointerPinchEndEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer hold begin event.
     *
     * This method should be integrated into LPointer::pointerHoldBeginEvent() to manage pointer hold begin events.
     *
     * @param event The pointer hold begin event to handle.
     */
    void handlePointerHoldBeginEvent(const LPointerHoldBeginEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle pointer hold end event.
     *
     * This method should be integrated into LPointer::pointerHoldEndEvent() to manage pointer hold end events.
     *
     * @param event The pointer hold end event to handle.
     */
    void handlePointerHoldEndEvent(const LPointerHoldEndEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle keyboard key event.
     *
     * This method should be integrated into LKeyboard::keyEvent() to handle key events.
     */
    void handleKeyboardKeyEvent(const LKeyboardKeyEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch down event.
     *
     * This method should be integrated into LTouch::touchDownEvent() to manage touch down events.
     *
     * @param event The touch down event to handle.
     * @param globalPos The event position transformed to global coordinates.
     */
    void handleTouchDownEvent(const LTouchDownEvent &event, const LPointF &globalPos, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch move event.
     *
     * This method should be integrated into LTouch::touchMoveEvent() to manage touch move events.
     *
     * @param event The touch move event to handle.
     * @param globalPos The event position transformed to global coordinates.
     */
    void handleTouchMoveEvent(const LTouchMoveEvent &event, const LPointF &globalPos, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch up event.
     *
     * This method should be integrated into LTouch::touchUpEvent() to manage touch up events.
     *
     * @param event The touch up event to handle.
     */
    void handleTouchUpEvent(const LTouchUpEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch frame event.
     *
     * This method should be integrated into LTouch::touchFrameEvent() to manage touch frame events.
     *
     * @param event The touch frame event to handle.
     */
    void handleTouchFrameEvent(const LTouchFrameEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Handle touch cancel event.
     *
     * This method should be integrated into LTouch::touchCancelEvent() to manage touch cancel events.
     *
     * @param event The touch cancel event to handle.
     */
    void handleTouchCancelEvent(const LTouchCancelEvent &event, EventOptionsFlags options = WaylandEvents | PointerConstraints | AuxFunc);

    /**
     * @brief Retrieve the main view of the scene.
     *
     * This method returns the main LSceneView associated with the LScene.
     *
     * @return A pointer to the main LSceneView of the scene.
     */
    LSceneView *mainView() const;

    /**
     * @brief Retrieve the view located at the specified position.
     *
     * This method returns the LView instance that occupies the given position within the scene.
     *
     * @param pos The position to query.
     * @param type The type of view to search for. Passing LView::Type::Undefined disables the filter.
     * @param filter Additional flags for searching only views with pointer and/or touch events enabled. 0 disables it.
     *
     * @return A pointer to the LView at the specified position, or nullptr if no view is found.
     */
    LView *viewAt(const LPoint &pos, LView::Type type = LView::Undefined, LBitset<InputFilter> filter = 0);

LPRIVATE_IMP_UNIQUE(LScene)
};

#endif // LSCENE_H
