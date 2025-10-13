#ifndef LSURFACE_H
#define LSURFACE_H

#include <CZ/Louvre/Private/LResourceRef.h>
#include <CZ/Louvre/LFactoryObject.h>
#include <CZ/Louvre/Layout/LSurfaceLayer.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RContentType.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/skia/core/SkRect.h>
#include <list>

namespace CZ
{
    class LSurfaceBuffer : public CZObjectBase
    {
    public:
        bool release() noexcept;
        LResourceRef buffer;
        CZWeak<LSurface> surface;
        std::weak_ptr<RImage> weakImage;
        std::shared_ptr<RDRMTimeline> acquireTimeline, releaseTimeline;
        UInt64 acquirePoint, releasePoint;
        std::shared_ptr<CZEventSource> acquireTimelineSource;
        bool attached;
        bool released;
        bool signaled;
        bool queued;
    };
};

/**
 * @brief A client "window"
 * 
 * @anchor lsurface_detailed
 *
 * In the context of Wayland, surfaces can be thought of as analogous to "application windows."
 * Each surface possesses attributes including position, size, buffer, input region, opaque region, damage region, and a designated role.
 * Clients are responsible for rendering the content of their windows onto a buffer and subsequently transmitting it to the compositor, which is then displayed on the screen.
 *
 * @section Roles
 *
 * Surfaces on their own lack functionality. This is where roles come into play, as they establish the guidelines for how the compositor
 * interacts with them, dictating their ordering, positioning, geometry interpretation, and more.\n
 *
 * Louvre currently implements the following roles:
 *
 * - LCursorRole (derived from the Wayland protocol)
 * - LDNDIconRole (derived from the Wayland protocol)
 * - LSubsurfaceRole (derived from the Wayland protocol)
 * - LPopupRole (derived from the XDG Shell protocol)
 * - LToplevelRole (derived from the XDG Shell protocol)
 * - LSessionLockRole (derived from the Session Lock protocol)
 * - LLayerRole (derived from the Layer Shell protocol)
 *
 * The surface's role can be accessed using the role() method or, if you already know the role in advance or wish to verify whether it matches one of them,
 * through the dedicated functions: cursorRole(), dndIcon(), popup(), toplevel(), subsurface(), etc.\n
 * Typically, once a role is assigned, it remains consistent throughout the surface's entire lifecycle.\n
 * You have the option to monitor changes in the surface's role by utilizing the roleChanged() event.\n
 *
 * To create additional roles beyond those offered by the library you can use the LBaseSurfaceRole class.
 *
 * @section Mapping
 *
 * In order to render a surface, several conditions must be met, such as having an assigned role and a non-null buffer().\n
 * To determine if a surface is renderable, you can use the mapped() property.
 * This property is `false` when the client wants to hide it through some rule of its role, or when
 * the necessary conditions for it to be rendered are not met.\n
 *
 * @section Callbacks
 *
 * To ensure that surfaces don't update their content more frequently than the refresh rate of the output (LOutput) they are displayed on,
 * or in scenarios where they are obscured by other surfaces, callbacks are employed.
 * Clients generate a callback resource and await the compositor's acknowledgment, signaling an optimal time to render the subsequent surface frame.
 * To acknowledge the callback of a surface, use requestNextFrame().
 *
 * @warning Calling requestNextFrame() clears the current damage region of the surface.
 *
 * @section Buffer
 *
 * The library automatically converts the surface's buffer into an object that can be used by the selected renderer.\n
 * Since the library currently only offers OpenGL ES 2.0 renderer, the buffer is converted to an OpenGL texture that can be accessed with texture().\n
 * It is also possible to access the native Wayland resource of the buffer with bufferResource().
 *
 * @section Damage
 *
 * To avoid the compositor redrawing the entire area of a surface on each frame, clients notify which regions have changed on their
 * buffers, known as damage. You can access this region with damage() or damageB() and be notified when it changes with the damageChanged() event.\n
 *
 * @note The default implementation of LOutput::paintGL() does not take into account the damage of surfaces, and therefore it renders in an inefficient way.
 *       It's recommended to use the LScene and LView system for rendering, which efficiently repaints only what is necessary during each frame.
 *       If you want to see an example of efficient rendering, check the [louvre-views](#louvre-views-example) or [louvre-weston-clone](#louvre-weston-clone-example) examples.
 *
 * @section Stacking Order
 *
 * Since Louvre v3, LCompositor::surfaces() no longer provides the stacking order of surfaces as defined by their roles.
 *
 * Instead, stacking should be determined using a combination of LCompositor::layers(), LSurface::subsurfacesAbove(),
 * LSurface::subsurfacesBelow(), and role-specific child lists, following a recursive approach as demonstrated in the default
 * implementation of LOutput::paintGL().
 *
 * @section Position
 *
 * One of the characteristics of Wayland is that clients have very little information and control over how their surfaces are positioned on the screen.\n
 * For this reason, the rules of their roles generally define it based on an offset relative to another surface, an output or a position given by you.\n
 * Louvre simplifies this by allowing you to assign the position of the surfaces with setPos() and access the position suggested by its role with rolePos().\n
 * In some cases, such as in the LPopupRole or LSubsurfaceRole role, the position set with setPos() is not taken into account.\n
 * You can see the positioning rules of each role in detail by viewing the documentation of rolePos() for each one.
 */
class CZ::LSurface : public LFactoryObject
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LSurface;

    /**
     * @brief ID of library roles
     *
     * ID of the current role of the surface accessible with roleId()
     */
    enum Role : UInt32
    {
        /// No role set
        Undefined = 0,

        /// LSubsurfaceRole
        Subsurface,

        /// LCursorRole
        Cursor,

        /// LPopupRole
        Popup,

        /// LDNDIconRole
        DNDIcon,

        /// LToplevelRole
        Toplevel,

        /// LSessionLockRole
        SessionLock,

        /// LLayerRole
        Layer
    };

    /**
     * @brief Pointer constraint modes.
     */
    enum PointerConstraintMode
    {
        /// No pointer constraint, the pointer is free to move anywhere.
        Free,

        /// Lock the pointer position somewhere inside pointerConstraintRegion().
        Lock,

        /// Confine the pointer to pointerConstraintRegion().
        Confine
    };

    /**
     * @brief Constructor of the LSurface class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LSurface(const void *params) noexcept;

    /**
     * @brief Destructor of the LSurface class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LSurface() noexcept;

    /**
     * @brief Prevents client commit requests from being applied immediately.
     *
     * While the lock is active, each client commit is stored in a queue
     * (including role state and any other double-buffered state) instead
     * of being applied right away.
     *
     * When the lock is released, the cached state is applied. If there are
     * other locks that were acquired earlier, the state will only be applied
     * once those locks are also released.
     *
     * For security reasons, even if the cached state could be applied
     * immediately, it is deferred. This is because client protocol errors
     * may be detected while applying commits, which can lead to resources
     * being unexpectedly destroyed.
     *
     * @note Locks created with LSurfacePrivate::lock() are applied immediately
     *       but are intended for internal use only.
     *
     * @return A shared pointer to the new surface lock.
     */
    std::shared_ptr<LSurfaceLock> lock() noexcept;

    /**
     * @brief Checks whether surface commits are currently locked.
     *
     * @see lock()
     */
    bool isLocked() const noexcept;

    /**
     * @brief Returns the list of child subsurfaces placed below this surface.
     *
     * The subsurfaces are ordered from background to foreground (back to front).
     */
    const std::vector<LSubsurfaceRole*>& subsurfacesBelow() const noexcept;

    /**
     * @brief Returns the list of child subsurfaces placed above this surface.
     *
     * The subsurfaces are ordered from background to foreground (back to front).
     */
    const std::vector<LSubsurfaceRole*>& subsurfacesAbove() const noexcept;

    /**
     * @brief Returns the surface's current layer.
     *
     * Surfaces without an assigned role default to the middle layer (LLayerMiddle).
     *
     * @see layerChanged()
     */
    LSurfaceLayer layer() const noexcept;

    /**
     * @brief Called when the surface's layer changes.
     *
     * The surface is automatically moved to the end of the new layer's list.
     */
    virtual void layerChanged() noexcept { repaintOutputs(); }

    /**
     * @brief Reinserts the surface at the end of its current layer.
     *
     * If the surface has a parent, the topmost parent is raised instead triggering topmostParent->raised().
     */
    void raise() noexcept;

    /**
     * @brief Called when the surface has been raised.
     */
    virtual void raised() noexcept { repaintOutputs(); }

    /**
     * @brief Returns the immediate parent surface, or nullptr if there is none.
     */
    LSurface* parent() const noexcept;

    /**
     * @brief Returns the topmost parent in the surface hierarchy, or nullptr if there is none.
     */
    LSurface* topmostParent() const noexcept;

    /**
     * @brief Checks whether any surface in the parent hierarchy has the specified role.
     *
     * @param role The role to check for.
     * @return true if any parent or ancestor surface has the given role, false otherwise.
     */
    bool isSubchildOf(Role role) const noexcept;

    /**
     * @brief Checks whether this surface is a child or subchild of the given surface.
     *
     * @param surface The surface to compare against.
     * @return true if this surface is a descendant of the given surface, false otherwise.
     */
    bool isSubchildOf(LSurface* surface) const noexcept;

    /**
     * @brief Called when the surface's parent has changed.
     */
    virtual void parentChanged() noexcept {}

    /**
     * @brief Returns the background blur controller associated with this surface.
     *
     * @return A pointer to the background blur controller. This is always valid, even if
     *         the client does not support the background blur protocol.
     */
    LBackgroundBlur* backgroundBlur() const noexcept;

    /**
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPos(SkIPoint newPos) noexcept;
    void setPos(SkPoint newPos) noexcept { setPos(newPos.x(), newPos.y()); };

    /**
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPos(Int32 x, Int32 y) noexcept;

    /**
     * @brief Assigns the x component of the position.
     *
     * Assigns the x component of the position of the surface.
     */
    void setX(Int32 x) noexcept;

    /**
     * @brief Assigns the y component of the position.
     *
     * Assigns the y component of the position of the surface.
     */
    void setY(Int32 y) noexcept;

    /**
     * @brief Position given by the compositor.
     *
     * Position of the surface assigned with setPos(), setX() or setY().
     */
    SkIPoint pos() const noexcept;

    /**
    * @brief Role position.
    *
    * Role position in surface coordinates. If the surface has no role, the same value as pos() is returned.
    */
    SkIPoint rolePos() const;

    /**
     * @brief Surface size in buffer coordinates.
     */
    SkISize sizeB() const noexcept;

    /**
     * @brief Surface size in surface coordinates.
     */
    SkISize size() const noexcept;

    /**
     * @brief Input region in surface coordinates.
     *
     * Region of the surface that is capable of receiving input, in surface coordinates.
     *
     * Already clipped by the surface bounds.
     */
    const SkRegion &inputRegion() const noexcept;

    bool inputRegionContainsGlobalPoint(SkIPoint point) const noexcept
    {
        const auto p { point - rolePos() };
        return inputRegion().contains(p.x(), p.y());
    }

    /**
     * @brief Opaque region in surface coordinates.
     *
     * Already clipped by the surface bounds.
     */
    const SkRegion &opaqueRegion() const noexcept;

    /**
     * @brief Invisible region in surface coordinates.
     *
     * Region within the surface that is completely transparent.
     * 
     * @note The invisible region protocol is experimental and thus not enabled by default in `LCompositor::createGlobalsRequest()`.
     *
     * Already clipped by the surface bounds.
     */
    const SkRegion &invisibleRegion() const noexcept;

    /**
     * @brief Damaged region in surface coordinates.
     *
     * Already clipped by the surface bounds.
     */
    const SkRegion &damage() const noexcept;

    /**
     * @brief Damaged region in buffer coordinates.
     *
     * Already clipped by the surface bounds.
     */
    const SkRegion &bufferDamage() const noexcept;

    /**
     * @brief Returns the content type that the surface represents.
     *
     * Clients using the Content Type Hint protocol can indicate the type of content a particular surface is displaying.\n
     * This information can be used, for example, to adapt the compositor behavior and to optimize the functioning
     * of hardware displays when assigned to outputs through LOutput::setContentType().
     *
     * The default value is @ref LContentTypeNone.
     *
     * @see LOutput::setContentType() and LSurface::contentTypeChanged()
     */
    RContentType contentType() const noexcept;

    /**
     * @brief Notify the client when the surface enters an output
     *
     * This method notifies the surface's client that the surface has become visible within the display area of a specific output.
     * The client application can use this information to synchronize the surface's scale with that of the output.
     * You can access the list of outputs where the surface is visible using the outputs() method.
     *
     * @note This method can be safely called multiple times with the same output as an argument because it internally checks whether the client has already been notified.
     *
     * @param output The output into which the surface has entered.
     */
    void sendOutputEnterEvent(LOutput *output) noexcept;

    /**
     * @brief Notify the client when the surface leaves an output
     *
     * This method informs the surface's client application that the surface is no longer visible on a particular output.
     * You can access the list of outputs where the surface is still visible using the outputs() method.
     *
     * @note This method can be safely called multiple times with the same output as an argument because it internally checks whether the client has already been notified.
     *
     * @param output The output from which the surface is no longer visible.
     */
    void sendOutputLeaveEvent(LOutput *output) noexcept;

    /**
     * @brief Surface intersected outputs
     *
     * Vector of output pointers in which the surface is visible, modifiable with the sendOutputEnterEvent() and sendOutputLeaveEvent() methods.
     */
    const std::unordered_set<LOutput*> &outputs() const noexcept;

    /**
     * @brief Repaints the intersected outputs
     *
     * Invokes the LOutput::repaint() method on all outputs listed in outputs().
     */
    void repaintOutputs() noexcept;

    /**
     * @brief Input capability
     *
     * Indicates whether the surface is able to receive pointer or touch input events.
     */
    bool receiveInput() const noexcept;

    /**
     * @brief Buffer scale
     *
     * Scale of the surface buffer. You can listen for changes to this property with the bufferScaleChanged() event.
     */
    Int32 scale() const noexcept;

    /**
     * @brief Gets the buffer transform of the surface.
     *
     * @return The buffer transform applied to the surface.
     */
    CZTransform bufferTransform() const noexcept;

    /**
     * @brief Gets the source rect of the surface in surface coordinates.
     *
     * For clients using the Viewporter protocol, a custom srcRect() detached from the buffer size
     * can be specified. For clients not using the protocol, the source rect covers the entire surface buffer.
     */
    const SkRect &srcRect() const noexcept;

    /**
     * @brief Check if the surface has pointer focus
     *
     * @return `true` if the surface has pointer focus, `false` otherwise.
     */
    bool hasPointerFocus() const noexcept;

    /**
     * @brief Check if the surface has keyboard focus
     *
     * @return `true` if the surface has keyboard focus, `false` otherwise.
     */
    bool hasKeyboardFocus() const noexcept;

    /**
     * @brief Check if the surface is grabbing the keyboard
     *
     * @return `true` if the surface is grabbing the keyboard, `false` otherwise.
     */
    bool hasKeyboardGrab() const noexcept;

    /**
     * @brief Image
     *
     * @warning It could return `nullptr` if the surface is not currently mapped.
     */
    std::shared_ptr<RImage> image() const noexcept;

    /**
     * @brief Native [wl_buffer](https://wayland.app/protocols/wayland#wl_buffer) handle
     *
     * Handle to the last commited Wayland buffer of the surface.
     *
     * @warning It could return `nullptr` if the surface is not currently mapped.
     */
    wl_buffer *bufferResource() const noexcept;

    /**
     * @brief Presence of damage
     *
     * Indicates if the surface has new damage since the last time the requestNextFrame() method was called.\n
     * You can access the damaged region with damage() or damageB().
     */
    bool hasDamage() const noexcept;

    /**
     * @brief Gets an ID that increments with each commit and new damage addition.
     *
     * The ID increments every time the surface is committed and new damage is added.
     *
     * @return The incremental damage ID of the surface.
     */
    UInt32 damageId() const noexcept;

    /**
     * @brief ACK the frame callback
     *
     * Notifies the surface that it's time for it to draw its next frame.\n
     * If not called, the given surface should not update its content.
     *
     * @warning This method clears the current damage region of the surface.
     */
    void requestNextFrame(bool clearDamage = true) noexcept;

    /**
     * @brief Mapped property
     *
     * Indicates if the surface can be rendered.
     */
    bool mapped() const noexcept;

    /**
     * @brief Gets the VSync preference of the client for this surface.
     *
     * @see LOutput::enableVSync()
     *
     * @return `true` if VSync is preferred, `false` if tearing is preferred.
    */
    bool prefersVSync() noexcept;

    /**
     * @brief Wayland surface resource
     *
     * Returns the resource generated by the [wl_surface](https://wayland.app/protocols/wayland#wl_surface) interface of the Wayland protocol.
     */
    Protocols::Wayland::RWlSurface *surfaceResource() const noexcept;

    /**
     * @brief Idle Inhibitor Resources
     */
    std::vector<Protocols::IdleInhibit::RIdleInhibitor*> idleInhibitorResources() const noexcept;

    /**
     * @brief Client owner of the surface.
     */
    LClient *client() const noexcept;

    /**
     * @name Roles
     *
     * Functionality related to roles.
     */

    ///@{

    /**
     * @brief ID of the role
     *
     * @returns The ID of the surface's role or LSurface::Undefined if it does not have a role.
     */
    Role roleId() const noexcept;

    /**
     * @brief Surface role
     *
     * @returns A pointer to the surface's role or `nullptr` if it does not have a role.
     */
    LBaseSurfaceRole *role() const noexcept;

    /**
     * @brief Cursor role
     *
     * @returns A pointer to an instance of LCursorRole or `nullptr` if it has a different role.
     */
    LCursorRole *cursorRole() const noexcept;

    /**
     * @brief Drag & Drop icon role
     *
     * @returns A pointer to an instance of LDNDIconRole or `nullptr` if it has a different role.
     */
    LDNDIconRole *dndIcon() const noexcept;

    /**
     * @brief Toplevel role
     *
     * @returns A pointer to an instance of LToplevelRole or `nullptr` if it has a different role.
     */
    LToplevelRole *toplevel() const noexcept;

    /**
     * @brief Popup role
     *
     * @returns A pointer to an instance of LPopupRole or `nullptr` if it has a different role.
     */
    LPopupRole *popup() const noexcept;

    /**
     * @brief Subsurface role
     *
     * @returns A pointer to an instance of LSubsurfaceRole or `nullptr` if it has a different role.
     */
    LSubsurfaceRole *subsurface() const noexcept;

    /**
     * @brief Session Lock role
     *
     * @returns A pointer to an instance of LSessionLockRole or `nullptr` if it has a different role.
     */
    LSessionLockRole *sessionLock() const noexcept;

    /**
     * @brief Layer role
     *
     * @returns A pointer to an instance of LLayerRole or `nullptr` if it has a different role.
     */
    LLayerRole *layerRole() const noexcept;

    ///@}

    /**
     * @anchor pointer_constraints
     *
     * @name Pointer Constraints
     *
     * Functionality related to pointer constraints.
     *
     * @see LPointer::pointerMoveEvent()
     */

    ///@{

    /**
     * @brief Current pointer constraint mode.
     *
     * Returns the current mode in which the client wants to constrain the pointer.
     *
     * @note The pointer constraint is not enabled automatically, see enablePointerConstraint().
     */
    PointerConstraintMode pointerConstraintMode() const noexcept;

    /**
     * @brief Invoked when pointerConstraintMode() changes.
     *
     * Each time the pointer constraint mode changes,
     * the pointer constraint is disabled and enablePointerConstraint()
     * must be called again to enable it.
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp pointerConstraintModeChanged
     */
    virtual void pointerConstraintModeChanged();

    /**
     * @brief Region within the surface where the pointer should be locked or confined if pointer constraint is enabled.
     *
     * Returns the region within the surface where the pointer should be locked
     * or confined if the pointer constraint is enabled.
     *
     * @return The region where the pointer should be constrained within the surface.
     * @see SkRegion::closestPointFrom()
     */
    const SkRegion &pointerConstraintRegion() const noexcept;

    /**
     * @brief Notifies a change in pointerConstraintRegion().
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp pointerConstraintRegionChanged
     */
    virtual void pointerConstraintRegionChanged();

    /**
     * @brief Notifies the client if the pointer is constrained.
     *
     * The surface must have pointer focus prior to calling this method
     * and have either a @ref Lock or @ref Confine @ref PointerConstraintMode,
     * otherwise, it is a no-op.
     *
     * @param enabled Boolean indicating if the pointer constraint is enabled.
     */
    void enablePointerConstraint(bool enabled);

    /**
     * @brief Indicates if the compositor enabled the pointer constraint for this surface.
     *
     * It is automatically set to `false` if the surface loses pointer focus
     * or the pointerConstraintMode() property changes.
     *
     * @see enablePointerConstraint()
     *
     * @return `true` if the pointer constraint is enabled for this surface, `false` otherwise.
     */
    bool pointerConstraintEnabled() const noexcept;

    /**
     * @brief Indicates where the pointer currently is within the surface if the pointer @ref Lock constrain mode is enabled.
     *
     * If pointerConstraintMode() is not @ref Lock or the client has never set this property,
     * it returns `(-1.f, -1.f)`.
     */
    SkPoint lockedPointerPosHint() const noexcept;

    /**
     * @brief Notifies a change in lockedPointerPosHint().
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp lockedPointerPosHintChanged
     */
    virtual void lockedPointerPosHintChanged();

    ///@}

/// @name Virtual Methods
/// @{

    /**
     * @brief Notifies about new damages on the surface
     *
     * Reimplement this virtual method if you want to be notified when the surface has new damage.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp damageChanged
     */
    virtual void damageChanged();

    /**
     * @brief Notifies a change in role.
     *
     * Indicates that the current `role()` has changed. Initially, the role is `nullptr`.
     *
     * The role always transitions either from `nullptr` to a valid role or from a role back to `nullptr`,
     * but never directly from one role (`roleA`) to another (`roleB`).
     *
     * During the execution of this method, the surface is always **unmapped**.
     *
     * @param prevRole Handle to the previous role, or `nullptr` if no role was assigned.
     * The current role is always obtained via `role()`.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp roleChanged
     */
    virtual void roleChanged() noexcept;

    /**
     * @brief Notifies of a change in the mapped() property.
     *
     * A surface is always initially unmapped. It becomes mapped when the client
     * assigns it a role, attaches a valid buffer (texture), and requests it to be presented.
     * If any of these conditions are not met, mapped() returns false.
     *
     * A surface is always unmapped before its role() is changed.
     *
     * @see mapped()
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp mappingChanged
     */
    virtual void mappingChanged();

    /**
     * @brief Notifies a change in buffer scale
     *
     * Reimplement this virtual method if you want to be notified when the surface's buffer scale changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp bufferScaleChanged
     */
    virtual void bufferScaleChanged();

    /**
     * @brief Notifies a change of the buffer transform
     *
     * Reimplement this virtual method if you want to be notified when the surface's buffer transform changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp bufferTransformChanged
     */
    virtual void bufferTransformChanged();

    /**
     * @brief Notifies a change in buffer dimensions
     *
     * Reimplement this virtual method if you want to be notified when the buffer size (sizeB()) changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp bufferSizeChanged
     */
    virtual void bufferSizeChanged();

    /**
     * @brief Notifies a change in the surface size
     *
     * Reimplement this virtual method if you want to be notified when the surface size() changes.
     *
     * @note This event differs from bufferSizeChanged(). The surface size() may change
     *       when the client applies a different scale factor, transform or sets a custom destination size
     *       while using the Viewport protocol.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp sizeChanged
     */
    virtual void sizeChanged();

    /**
     * @brief Notifies a change in the src rect
     *
     * Reimplement this virtual method if you want to be notified when srcRect() changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp srcRectChanged
     */
    virtual void srcRectChanged();

    /**
     * @brief Notifies of a change in the opaque region
     *
     * Reimplement this virtual method if you wish to be notified when the surface changes its opaque region.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp opaqueRegionChanged
     */
    virtual void opaqueRegionChanged();

    /**
     * @brief Notifies of a change in the invisible region
     *
     * Reimplement this virtual method if you wish to be notified when the surface changes its invisible region.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp invisibleRegionChanged
     */
    virtual void invisibleRegionChanged();

    /**
     * @brief Notifies of a change in the input region
     *
     * Reimplement this virtual method if you want to be notified when the surface changes its input region.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp inputRegionChanged
     */
    virtual void inputRegionChanged();

    /**
     * @brief Request a repaint of the surface
     *
     * This request can be initiated either by the library or by the client, serving as an explicit command to repaint the surface.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp requestedRepaint
     */
    virtual void requestedRepaint();

    /**
     * @brief Notifies about changes in the VSync preference
     *
     * This event is triggered when the prefersVSync() property changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp prefersVSyncChanged
     */
    virtual void prefersVSyncChanged();

    /**
     * @brief Notifies a change of content type
     *
     * This event is triggered when the contentType() property changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp contentTypeChanged
     */
    virtual void contentTypeChanged();

    virtual void commitEvent(const LSurfaceCommitEvent &e) noexcept;
    virtual bool event(const CZEvent &e) noexcept override;

/// @}

    LPRIVATE_IMP_UNIQUE(LSurface)
};

#endif // LSURFACE_H
