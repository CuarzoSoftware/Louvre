#ifndef LSURFACE_H
#define LSURFACE_H

#include <LObject.h>
#include <LTexture.h>
#include <LRegion.h>
#include <LRect.h>

#include <list>
#include <mutex>

/*!
 * @brief A client "Window"
 *
 * In the context of Wayland, surfaces can be thought of as analogous to "application windows."
 * Each surface possesses attributes including position, size, buffer, input region, opaque region, damage region, and a designated role.
 * Clients are responsible for rendering the content of their windows onto a buffer and subsequently transmitting it to the compositor, which is then displayed on the screen.
 *
 * @section Roles
 *
 * Surfaces on their own lack functionality. This is where roles come into play, as they establish the guidelines for how the compositor
 * interacts with surfaces, dictating their ordering, positioning, geometry interpretation, and more.\n
 *
 * The library currently implements the following roles:
 *
 * - LCursorRole (derived from the Wayland protocol)
 * - LDNDIconRole (derived from the Wayland protocol)
 * - LSubsurfaceRole (derived from the Wayland protocol)
 * - LPopupRole (derived from the XDG Shell protocol)
 * - LToplevelRole (derived from the XDG Shell protocol)
 *
 * The surface's role can be accessed using the role() function or, if you already know the role in advance or wish to verify whether it matches one of them,
 * through the dedicated functions: cursorRole(), dndIcon(), popup(), toplevel(), and subsurface().\n
 * Typically, once a role is assigned, it remains consistent throughout the surface's entire lifecycle.\n
 * You have the option to monitor changes in the surface's role by utilizing the roleChanged() event.\n
 *
 * To create additional roles beyond those offered by the library you can use the LBaseSurfaceRole class.
 *
 * @section Mapping
 *
 * In order to render a surface, several conditions must be met, such as having an assigned role and a non-null buffer.\n
 * To determine if a surface is renderable, you can use the mapped() property.
 * This property is `false` when the client wants to hide it through some rule of its role, or when
 * the necessary conditions for it to be rendered are not met.\n
 *
 * @section Callbacks
 *
 * To ensure that surfaces don't update their content more frequently than the refresh rate of the output (LOutput) they are displayed on,
 * or in scenarios where they are obscured by other surfaces, callbacks are employed.\n
 * Clients generate a callback resource and await the compositor's acknowledgment, signaling an optimal time to render the subsequent surface frame.\n
 * To ACK the callback of a surface, use requestNextFrame().
 *
 * @warning Calling requestNextFrame() clears the current damage region of the surface.
 *
 * @section Buffer
 *
 * The library automatically converts the surface's buffer into an object that can be used by the selected renderer.\n
 * Since the library currently only offers OpenGL ES 2.0 renderer, the buffer is converted to an OpenGL texture that can be accessed with texture().\n
 * It is also possible to access the native Wayland resource of the buffer with buffer().
 *
 * @section Damage
 *
 * To avoid the compositor redrawing the entire area of a surface on each frame, clients notify which regions have changed on their
 * buffers, known as damage. You can access this region with damage() or damageB() and be notified when it changes with the damaged() event.\n
 * The default implementation of LOutput::paintGL() does not take into account the damage of surfaces, and therefore it renders in an inefficient way.
 * If you want to see an example of efficient rendering, check the **louvre-views** or **louvre-weston-clone** examples.
 *
 * @section Order
 *
 * The library uses a list to keep track of all the surfaces created by clients, which is accessible through LCompositor::surfaces().\n
 * This list maintains the order of the surfaces on the Z-axis defined by the protocols of their roles. Nevertheless, it is possible to reorder the surfaces respecting these hierarchies with the raise() method.\n
 * The surfaces are rendered by default following the order of the list, so the first ones are located in the background and the last ones in the front.\n
 * If you want to use your own list, you can make use of the virtual constructor and destructor of LSurface (LCompositor::createSurfaceRequest() and LCompositor::destroySurfaceRequest() respectively)
 * to listen when clients create or destroys a surface.\n
 *
 * @note If you want to use your own list of surfaces, you should keep in mind that some roles require a specific order to be able to function, so you may need to study the rules of their protocols.
 *
 * @section Position
 *
 * One of the characteristics of Wayland is that clients have very little information and control over how their surfaces are positioned on the screen.\n
 * For this reason, the rules of their roles generally define it based on an offset relative to another surface or a position given by
 * the compositor itself.\n
 * The library simplifies this problem by allowing you to assign the position of the surfaces with setPos() and access the position suggested by its role with
 * rolePos().\n
 * In some cases, such as in the LPopupRole or LSubsurfaceRole role, the position given by the compositor is not taken into account.\n
 * You can see the positioning rules of each role in detail by viewing the documentation of rolePos() for each one.
 */
class Louvre::LSurface : public LObject
{
public:

    struct Params;

    /*!
     * @brief ID of library roles
     *
     * ID of the current role of the surface accessible with roleId()
     */
    enum Role : UInt32
    {
        /// No role set
        Undefined = 0,

        /// LToplevelRole
        Toplevel = 1,

        /// LPopupRole
        Popup = 2,

        /// LSubsurfaceRole
        Subsurface = 3,

        /// LCursorRole
        Cursor = 4,

        /// LDNDIconRole
        DNDIcon = 5
    };

    /*!
     * @brief ID of the role
     *
     * @returns The ID of the surface's role or LSurface::Undefined if it does not have a role.
     */
    Role roleId() const;

    /*!
     * @brief Surface role
     *
     * @returns A pointer to the surface's role or `nullptr` if it does not have a role.
     */
    LBaseSurfaceRole *role() const;

    /*!
     * @brief Cursor role
     *
     * @returns A pointer to an instance of LCursorRole or `nullptr` if it has a different role.
     */
    LCursorRole *cursorRole() const;

    /*!
     * @brief Drag & Drop icon role
     *
     * @returns A pointer to an instance of LDNDIconRole or `nullptr` if it has a different role.
     */
    LDNDIconRole *dndIcon() const;

    /*!
     * @brief Toplevel role
     *
     * @returns A pointer to an instance of LToplevelRole or `nullptr` if it has a different role.
     */
    LToplevelRole *toplevel() const;

    /*!
     * @brief Popup role
     *
     * @returns A pointer to an instance of LPopupRole or `nullptr` if it has a different role.
     */
    LPopupRole *popup() const;

    /*!
     * @brief Subsurface role
     *
     * @returns A pointer to an instance of LSubsurfaceRole or `nullptr` if it has a different role.
     */
    LSubsurfaceRole *subsurface() const;

    /*!
     * @brief Constructor of the LSurface class.
     *
     * @param params Internal parameters of the library provided in the virtual constructor LCompositor::createSurfaceRequest().
     */
    LSurface(Params *params);

    /*!
     * @brief Destructor of the LSurface class.
     *
     * Invoked after LCompositor::destroySurfaceRequest().
     */
    virtual ~LSurface();

    /// @cond OMIT
    LSurface(const LSurface&) = delete;
    LSurface& operator= (const LSurface&) = delete;
    /// @endcond

    /*!
     * @brief Position given by the compositor.
     *
     * Position of the surface assigned with setPos(), setX() or setY().
     */
    const LPoint &pos() const;

    /*!
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPos(const LPoint &newPos);

    /*!
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPos(Int32 x, Int32 y);

    /*!
     * @brief Assigns the x component of the position.
     *
     * Assigns the x component of the position of the surface.
     */
    void setX(Int32 x);

    /*!
     * @brief Assigns the y component of the position.
     *
     * Assigns the y component of the position of the surface.
     */
    void setY(Int32 y);

    /*!
    * @brief Role position.
    *
    * Role position in surface coordinates. If the surface has no role, the same value as pos() is returned.
    */
    const LPoint &rolePos() const;

    /*!
     * @brief Surface size in buffer coordinates.
     */
    const LSize &sizeB() const;

    /*!
     * @brief Surface size in surface coordinates.
     */
    const LSize &size() const;

    /*!
     * @brief Input region in surface coordinates.
     *
     * Region of the surface that is capable of receiving input, in surface coordinates.
     */
    const LRegion &inputRegion() const;

    /*!
     * @brief Opaque region in surface coordinates.
     */
    const LRegion &opaqueRegion() const;

    /*!
     * @brief Translucent region in surface coordinates.
     *
     * Translucent region in surface coordinates (inverted opaque region).
     */
    const LRegion &translucentRegion() const;

    /*!
     * @brief Damaged region in surface coordinates.
     */
    const LRegion &damage() const;

    /*!
     * @brief Damaged region in buffer coordinates.
     */
    const LRegion &damageB() const;

    /*!
     * @brief Notify when the surface enters an output
     *
     * This function notifies the surface's client that the surface has become visible within the display area of a specific output.
     * The client application can use this information to synchronize the surface's scale with that of the output.
     * You can access the list of outputs where the surface is visible using the outputs() method.
     *
     * @param output The output into which the surface has entered.
     */
    void sendOutputEnterEvent(LOutput *output);

    /*!
     * @brief Notify when the surface leaves an output
     *
     * This function informs the surface's client application that the surface is no longer visible on a particular output.
     * You can access the list of outputs where the surface is still visible using the outputs() method.
     *
     * @param output The output from which the surface is no longer visible.
     */
    void sendOutputLeaveEvent(LOutput *output);

    /*!
     * @brief Surface intersected outputs
     *
     * List of outputs in which the surface is visible, modifiable with the sendOutputEnterEvent() and sendOutputLeaveEvent() methods.
     */
    const std::list<LOutput*>&outputs() const;

    /*!
     * @brief Repaints the intersected outputs
     *
     * Invokes the LOutput::repaint() method on all outputs listed in outputs().
     */
    void repaintOutputs();

    /*!
     * @brief Minimized property
     *
     * Stores the minimized state of the surface set with setMinimized().
     *
     * @returns `true` if the surface is minimized and `false` otherwise.
     */
    bool minimized() const;

    /*!
     * @brief Sets the minimized property
     *
     * Minimize/unminimize the surface and all its children.
     */
    void setMinimized(bool state);

    /*!
     * @brief Input capability
     *
     * Indicates whether the surface is able to receive pointer or touch input events.
     */
    bool receiveInput() const;

    /*!
     * @brief Buffer scale
     *
     * Scale of the surface buffer. You can listen for changes to this property with the bufferScaleChanged() event.
     */
    Int32 bufferScale() const;

    /*!
     * @brief OpenGL texture
     *
     * Representation of the surface's buffer as an OpenGL texture.\n
     *
     * @warning It could return `nullptr` if the surface is not currently mapped.
     */
    LTexture *texture() const;

    /*!
     * @brief Native wl_buffer handle
     *
     * Handle to the last commited Wayland buffer of the surface.
     */
    wl_buffer *buffer() const;

    /*!
     * @brief Presence of damage
     *
     * Indicates if the surface has new damage since the last time the requestNextFrame() method was called.\n
     * You can access the damaged region with damage() or damageB().
     */
    bool hasDamage() const;

    /*!
     * @brief Get an ID that increments with each commit and new damage addition.
     *
     * The ID increments every time the surface is committed and new damage is added.
     *
     * @return The incremental damage ID of the surface.
     */
    UInt32 damageId() const;

    /*!
     * @brief ACK the frame callback
     *
     * Notifies the surface that it's time for it to draw its next frame.\n
     * If not called, the given surface should not update its content.
     *
     * @warning This method clears the current damage region of the surface.
     */
    void requestNextFrame(bool clearDamage = true);

    /*!
     * @brief Mapped property
     *
     * Indicates if the surface can be rendered.
     */
    bool mapped() const;

    /*!
     * @brief Wayland surface resource
     *
     * Returns the resource generated by the [wl_surface](https://wayland.app/protocols/wayland#wl_surface) interface of the Wayland protocol.
     */
    Protocols::Wayland::RSurface *surfaceResource() const;

    /*!
     * @brief Client owner of the surface.
     */
    LClient *client() const;

    /*!
     * @brief Parent surface
     *
     * @returns A pointer to the parent surface or `nullptr` if it does not have a parent.
     */
    LSurface *parent() const;

    /*!
     * @brief Topmost parent of the surface.
     *
     * @returns A pointer to the topmost parent of the surface, or `nullptr` if it does not have a parent.
     */
    LSurface *topmostParent() const;

    /*!
     * @brief Child surfaces.
     *
     * List of child surfaces.
     */
    const std::list<LSurface*>&children() const;

    /*!
     * @brief Check if the surface is a subchild of a popup.
     *
     * This function determines whether the surface is a subchild of a popup surface.
     * It returns `true` if the surface is a subchild of a popup surface in the hierarchy; otherwise, it returns `false`.
     *
     * @return `true` if the surface is a subchild of a popup surface, `false` otherwise.
     */
    bool isPopupSubchild() const;

    /*!
     * @brief Check if the surface has a subchild popup.
     *
     * This function checks whether the surface has a subchild popup surface.
     * It returns `true` if the surface has a popup subchild; otherwise, it returns `false`.
     *
     * @return `true` if the surface has a subchild popup, `false` otherwise.
     */
    bool hasPopupSubchild() const;

    /*!
     * @brief Check if the surface is a subchild of the specified parent surface.
     *
     * This function checks whether the surface is a subchild of the provided parent surface.
     *
     * @param parent A pointer to the potential parent LSurface to check against.
     * @return `true` if the surface is a subchild of the provided parent surface, `false` otherwise.
     */
    bool isSubchildOf(LSurface *parent) const;

    /*!
     * @brief Moves a surface to the top of the compositor's surfaces list
     *
     * This function repositions the specified surface at the end of the surfaces list managed by the compositor.
     * As a result, the surface should be rendered last in a frame, effectively placing it in front of all other surfaces.
     * If the surface being raised is a parent of other surfaces, those child surfaces will also be raised,
     * ensuring that the existing hierarchical order required by certain protocols is maintained.
     */
    void raise();

    /*!
     * @brief Retrieve the previous surface in the compositor surfaces list (LCompositor::surfaces()).
     *
     * This function returns a pointer to the surface that precedes the current surface in the compositor's surfaces list.
     * If the current surface is the first one in the list, the function returns `nullptr`.
     *
     * @return A pointer to the previous LSurface or `nullptr` if the current surface is the first in the list.
     */
    LSurface *prevSurface() const;

    /*!
     * @brief Retrieve the next surface in the compositor surfaces list (LCompositor::surfaces()).
     *
     * This function returns a pointer to the surface that follows the current surface in the compositor's surfaces list.
     * If the current surface is the last one in the list, the function returns `nullptr`.
     *
     * @return A pointer to the next LSurface or `nullptr` if the current surface is the last in the list.
     */
    LSurface *nextSurface() const;

/// @name Virtual Methods
/// @{

    /*!
     * @brief Notifies about new damages on the surface
     *
     * Reimplement this virtual method if you want to be notified when the surface has new damages.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp damaged
     */
    virtual void damaged();

    /*!
     * @brief Notifies a change of role
     *
     * Reimplement this virtual method if you want to be notified when the surface changes its role.
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp roleChanged
     */
    virtual void roleChanged();

    /*!
     * @brief Notifies a change of parent
     *
     * Reimplement this virtual method if you want to be notified when the surface changes its parent.
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp parentChanged
     */
    virtual void parentChanged();

    /*!
     * @brief Notifies a change in the mapping state
     *
     * Reimplement this virtual method if you want to be notified when the surface changes its mapping state.
     *
     * #### Default implementation
     * @snippet LSurfaceDefault.cpp mappingChanged
     */
    virtual void mappingChanged();

    /*!
     * @brief Notifies a change in buffer scale
     *
     * Reimplement this virtual method if you want to be notified when the surface's buffer scale changes.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp bufferScaleChanged
     */
    virtual void bufferScaleChanged();

    /*!
     * @brief Notifies a change in buffer dimensions
     *
     * Reimplement this virtual method if you want to be notified when the buffer of the surface changes in size.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp bufferSizeChanged
     */
    virtual void bufferSizeChanged();

    /*!
     * @brief Notifies of a change in the opaque region
     *
     * Reimplement this virtual method if you wish to be notified when the surface changes its opaque region.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp opaqueRegionChanged
     */
    virtual void opaqueRegionChanged();

    /*!
     * @brief Notifies of a change in the input region
     *
     * Reimplement this virtual method if you want to be notified when the surface changes its input region.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp inputRegionChanged
     */
    virtual void inputRegionChanged();

    /*!
     * @brief Notifies when the surface is elevated
     *
     * If you want to receive notifications when the surface is repositioned to the top of the compositor's list of surfaces,
     * you can override this virtual method.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp raised
     */
    virtual void raised();

    /*!
     * @brief Notifies when the surface changes its position in the surfaces list
     *
     * Override this virtual method if you wish to be informed about changes in the order of the surface within
     * the compositor's list of surfaces.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp orderChanged
     */
    virtual void orderChanged();

    /*!
     * @brief Request a repaint of the surface
     *
     * This request can be initiated either by the library or by the client, serving as an explicit command to repaint the surface.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp requestedRepaint
     */
    virtual void requestedRepaint();

    /*!
     * @brief Notifies about changes in the minimized state of the surface
     *
     * This method is called to signal changes in the minimized state of the surface.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp minimizedChanged
     */
    virtual void minimizedChanged();
/// @}

    LPRIVATE_IMP(LSurface)
};

#endif // LSURFACE_H
