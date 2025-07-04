#ifndef LPOINTER_H
#define LPOINTER_H

#include <LPointerButtonEvent.h>
#include <LFactoryObject.h>
#include <LPoint.h>
#include <linux/input-event-codes.h>
#include <memory>
#include <vector>

/**
 * @brief Class for handling events generated by pointing devices.
 *
 * @anchor lpointer_detailed
 *
 * The LPointer class allows you to listen to input events generated by devices such as a mouse or touchpad and redirect them to client surfaces.
 * There is a single instance of LPointer, which can be accessed through LSeat::pointer().
 *
 * ### Wayland Events
 *
 * To send pointer events to clients, you must first assign focus to a surface using one of the setFocus() variants.\n
 * Subsequently, all pointer events sent with any of the **sendXXXEvent()** methods are directed to the currently focused surface.
 */
class Louvre::LPointer : public LFactoryObject
{
public:

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LPointer;

    /**
     * @brief LPointer class constructor.
     *
     * There is a single instance of LPointer, which can be accessed from LSeat::pointer().
     *
     * @param params Internal library parameters provided in the LCompositor::createObjectRequest() virtual constructor.
     */
    LPointer(const void *params) noexcept;

    /**
     * @brief LPointer class destructor.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LPointer();

    LCLASS_NO_COPY(LPointer)

    /**
     * @brief Gets the focused surface.
     *
     * This method returns the surface that has been assigned pointer focus using setFocus().
     *
     * @note Only the focused surface can receive pointer events.
     *
     * @return The focused surface, or `nullptr` if no surface has pointer focus.
     */
    LSurface *focus() const noexcept;

    /**
     * @brief Sets the pointer focus.
     *
     * This method assigns the pointer focus to the specified surface at the given local surface position within the surface.
     *
     * If another surface already has pointer focus, it will lose it.
     *
     * @param surface Surface to which the pointer focus will be assigned, or `nullptr` to remove focus from all surfaces.
     * @param localPos Local position within the surface where the pointer enters.
     */
    void setFocus(LSurface *surface, const LPoint &localPos) noexcept;

    /**
     * @brief Sets the pointer focus.
     *
     * Sets the pointer focus to the provided surface based on the current LCursor position.
     *
     * @note This method internally transforms the LCursor position to the local coordinates of the focused surface,
     *       taking into account the surface's role position.
     *
     * @param surface The surface to which you want to assign the pointer focus or `nullptr` to remove focus from all surfaces.
     */
    void setFocus(LSurface *surface) noexcept;

    /**
     * @brief Keep track of the surface pressed by the main pointer button.
     *
     * This is just a utility used by the default LPointer implementation to ensure that pointer focus remains
     * on a surface while it's being actively pressed by the left pointer button, for example, during text selection, even
     * if the pointer moves outside the surface boundaries.\n
     *
     * @note This is unrelated to drag & drop sessions.
     *
     * @param surface The surface being pressed or `nullptr` to unset.
     *
     * @see draggingSurface()
     */
    void setDraggingSurface(LSurface *surface) noexcept;

    /**
     * @brief Surface being actively pressed by the main pointer button.
     *
     * This method returns the surface that is currently being actively pressed by the main pointer button.
     *
     * @returns The surface being pressed or `nullptr`.
     *
     * @see setDraggingSurface()
     */
    LSurface *draggingSurface() const noexcept;

    /**
     * @brief Looks for a surface at the given position.
     *
     * This method looks for the first mapped surface that contains the point given point.\n
     * It takes into account the surfaces rolePos(), size(), inputRegion() and the reverse order
     * given by the LCompositor::surfaces() list.
     *
     * @note Some surface roles do not have an input region such as LCursorRole or LDNDIconRole so these surfaces are always ignored.
     *
     * @param point Point in compositor-global coordinates.
     * @returns Returns the first surface that contains the point or `nullptr` if no surface is found.
     */
    LSurface *surfaceAt(const LPoint &point);

    /**
     * @brief Vector of all currently pressed buttons.
     *
     * @see isButtonPressed()
     */
    const std::vector<LPointerButtonEvent::Button> &pressedButtons() const noexcept;

    /**
     * @brief Checks if a button is pressed.
     *
     * @see pressedKeys()
     */
    bool isButtonPressed(LPointerButtonEvent::Button button) const noexcept;

    /**
     * @name Client Events
     *
     * These methods allow you to send pointer events to clients.
     *
     * @note Pointer events are sent to the currently focused surface set with setFocus(). If no surface has focus, calling these methods has no effect.
     */

///@{

    /**
     * @brief Sends a pointer move event to the currently focused surface.
     *
     * @note To specify the position within the surface modify the mutable @ref LPointerMoveEvent::localPos property.
     */
    void sendMoveEvent(const LPointerMoveEvent &event);

    /**
     * @brief Sends a pointer button event to the currently focused surface.
     */
    void sendButtonEvent(const LPointerButtonEvent &event);

    /**
     * @brief Sends a scroll event to the currently focused surface.
     *
     * @see enableNaturalScrollingX() and enableNaturalScrollingY().
     */
    void sendScrollEvent(const LPointerScrollEvent &event);

    /**
     * @brief Sends a swipe begin gesture event to the currently focused surface.
     */
    void sendSwipeBeginEvent(const LPointerSwipeBeginEvent &event);

    /**
     * @brief Sends a swipe update gesture event to the currently focused surface.
     *
     * @note A sendSwipeBeginEvent() should have been sent before, otherwise this is a no-op.
     */
    void sendSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event);

    /**
     * @brief Sends a swipe end gesture event to the currently focused surface.
     *
     * @note A sendSwipeBeginEvent() should have been sent before, otherwise this is a no-op.\n
     *       This event is automatically sent if a sendSwipeBeginEvent() was sent and the surface lost focus.
     */
    void sendSwipeEndEvent(const LPointerSwipeEndEvent &event);

    /**
     * @brief Sends a pinch begin gesture event to the currently focused surface.
     */
    void sendPinchBeginEvent(const LPointerPinchBeginEvent &event);

    /**
     * @brief Sends a pinch update gesture event to the currently focused surface.
     *
     * @note A sendPinchBeginEvent() should have been sent before, otherwise this is a no-op.
     */
    void sendPinchUpdateEvent(const LPointerPinchUpdateEvent &event);

    /**
     * @brief Sends a pinch end gesture event to the currently focused surface.
     *
     * @note A sendPinchBeginEvent() should have been sent before, otherwise this is a no-op.\n
     *       This event is automatically sent if a sendPinchBeginEvent() was sent and the surface lost focus.
     */
    void sendPinchEndEvent(const LPointerPinchEndEvent &event);

    /**
     * @brief Sends a hold begin gesture event to the currently focused surface.
     */
    void sendHoldBeginEvent(const LPointerHoldBeginEvent &event);

    /**
     * @brief Sends a hold end gesture event to the currently focused surface.
     *
     * @note A sendHoldBeginEvent() should have been sent before, otherwise this is a no-op.\n
     *       This event is automatically sent if a sendHoldBeginEvent() was sent and the surface lost focus.
     */
    void sendHoldEndEvent(const LPointerHoldEndEvent &event);

///@}

    /**
     * @name Virtual Methods
     */

///@{

    /**
     * @brief Pointer move event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerMoveEvent
     */
    virtual void pointerMoveEvent(const LPointerMoveEvent &event);

    /**
     * @brief Pointer button event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerButtonEvent
     */
    virtual void pointerButtonEvent(const LPointerButtonEvent &event);

    /**
     * @brief Pointer scroll event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerScrollEvent
     */
    virtual void pointerScrollEvent(const LPointerScrollEvent &event);

    /**
     * @brief Pointer swipe begin gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerSwipeBeginEvent
     */
    virtual void pointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event);

    /**
     * @brief Pointer swipe update gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerSwipeUpdateEvent
     */
    virtual void pointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event);

    /**
     * @brief Pointer swipe end gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerSwipeEndEvent
     */
    virtual void pointerSwipeEndEvent(const LPointerSwipeEndEvent &event);

    /**
     * @brief Pointer pinch begin gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerPinchBeginEvent
     */
    virtual void pointerPinchBeginEvent(const LPointerPinchBeginEvent &event);

    /**
     * @brief Pointer pinch update gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerPinchUpdateEvent
     */
    virtual void pointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event);

    /**
     * @brief Pointer pinch end gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerPinchEndEvent
     */
    virtual void pointerPinchEndEvent(const LPointerPinchEndEvent &event);

    /**
     * @brief Pointer hold begin gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerHoldBeginEvent
     */
    virtual void pointerHoldBeginEvent(const LPointerHoldBeginEvent &event);

    /**
     * @brief Pointer hold end gesture event generated by the input backend.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp pointerHoldEndEvent
     */
    virtual void pointerHoldEndEvent(const LPointerHoldEndEvent &event);

    /**
     * @brief Set cursor request.
     *
     * Triggered when a client requests to set the cursor texture, hotspot or hide it.
     *
     * @see LCursor::setCursor()
     * @see LClient::lastCursorRequest().
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp setCursorRequest
     */
    virtual void setCursorRequest(const LClientCursor &clientCursor);

    /**
     * @brief Notifies when the focused surface changes.
     *
     * This method is triggered whenever the focus() property changes, either by
     * calling setFocus() or when the currently focused surface is destroyed.
     *
     * @warning Changing the focused surface within this event using setFocus()
     *          may result in an infinite feedback loop if not managed carefully.
     *
     * #### Default Implementation
     * @snippet LPointerDefault.cpp focusChanged
     */
    virtual void focusChanged();

///@}

    LPRIVATE_IMP_UNIQUE(LPointer)
};

#endif // LPOINTER_H
