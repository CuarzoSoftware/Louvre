#ifndef LSURFACE_H
#define LSURFACE_H

#include <LObject.h>
#include <LTexture.h>
#include <LRegion.h>
#include <LRect.h>

#include <list>
#include <mutex>

using namespace std;

/*!
 * @brief A client "Window"
 *
 * Surfaces in Wayland are typically a representation of what is commonly known as "an application window".\n
 * They have a position, size, buffer, input region, opaque region, damage region, and a role.\n
 * Clients draw the content of their windows in a buffer and then send it to the compositor to display it on the screen.\n
 *
 * @section Roles
 *
 * Surfaces by themselves lack sufficient functionality to be displayed on the screen. That's why there are roles that 
 * define the rules on how to order them, position them, interpret their geometries, etc.\n
 *
 * The library currently offers the following roles:
 *
 * * LCursorRole
 * * LDNDIconRole
 * * LPopupRole
 * * LToplevelRole
 * * LSubsurfaceRole
 *
 * You can access the surface's role with role() or with cursor(), dndIcon(), popup(), toplevel() and subsurface() if you know it in advance or
 * want to check if it is one of them.\n
 * Generally, once a role is assigned it is maintained throughout the entire life cycle of the surface.\n
 * You can listen when the surface changes role with roleChanged().\n
 *
 * To create additional roles beyond those offered by the library you can use the LBaseSurfaceRole class.
 *
 * @section Mapping
 *
 * In order to render a surface, several conditions must be met, such as having an assigned role and a non-null buffer.\n
 * To determine if a surface is renderable, you can use the mapped() property. 
 * This property is false when the client wants to hide it through some rule of its role, or when 
 * the necessary conditions for it to be rendered are not met.\n
 *
 * @section Callbacks
 *
 * To prevent surfaces from updating their content at a rate higher than that of the output (LOutput) on which they are visible or in cases where they are hidden by other surfaces, callbacks are used.\n
 * Clients create a callback resource and wait for the compositor to respond when it is a good time for them to render the next frame of the surface.\n
 * To return the callback of a surface, use requestNextFrame().
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
 * buffers, known as damages. You can access this region with damagesB() or damagesC() and be notified when it changes with damaged().\n
 * The default implementation of LOutput::paintGL() does not take into account the damages of surfaces, and therefore it renders in an inefficient way. If you want to see an example of efficient rendering, you can study the code of the sample compositor **louvre-weston-clone**.
 *
 * @section Order
 *
 * The library uses a list to keep track of all the surfaces created by clients, which is accessible through LCompositor::surfaces().\n
 * This list maintains the order of the surfaces on the Z-axis defined by the protocols of their roles. Nevertheless, it is possible to reorder the surfaces respecting these hierarchies with LCompositor::raiseSurface().\n
 * The surfaces are rendered by default following the order of the list, so the first ones are located in the background and the last ones in the front.\n
 * If you want to use your own list, you can make use of the virtual constructor and destructor of LSurface (LCompositor::createSurfaceRequest() and LCompositor::destroySurfaceRequest() respectively)
 * to listen when clients create or delete a surface.\n
 *
 * @warning If you want to use your own list of surfaces, you should keep in mind that some roles require a specific order to be able to function, so you may need to study the rules of their protocols.
 *
 * @section Position
 *
 * One of the characteristics of Wayland is that clients have very little information and control over how their surfaces are positioned on the screen.\n
 * For this reason, the rules of their roles generally define it based on an offset relative to another surface or a position given by
 * the compositor itself.\n
 * The library simplifies this problem by allowing you to assign the position of the surfaces with setPosC() and access the position suggested by its role with
 * rolePosC().\n
 * In some cases, such as in the LPopupRole or LSubsurfaceRole role, the position given by the compositor is not taken into account.\n
 * You can see the positioning rules of each role in detail by viewing the documentation of rolePosC() for each one.
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
     * @returns The ID of the surface's role or Role::Undefined if it does not have a role.
     */
    Role roleId() const;

    /*!
     * @brief Surface role
     *
     * @returns A pointer to the surface's role or nullptr if it does not have a role.
     */
    LBaseSurfaceRole *role() const;

    /*!
     * @brief Cursor role
     *
     * @returns A pointer to an instance of LCursorRole or nullptr if it has a different role.
     */
    LCursorRole *cursorRole() const;

    /*!
     * @brief Drag & Drop icon role
     *
     * @returns A pointer to an instance of LDNDIconRole or nullptr if it has a different role.
     */
    LDNDIconRole *dndIcon() const;

    /*!
     * @brief Toplevel role
     *
     * @returns A pointer to an instance of LToplevelRole or nullptr if it has a different role.
     */
    LToplevelRole *toplevel() const;

    /*!
     * @brief Popup role
     *
     * @returns A pointer to an instance of LPopupRole or nullptr if it has a different role.
     */
    LPopupRole *popup() const;

    /*!
     * @brief Subsurface role
     *
     * @returns A pointer to an instance of LSubsurfaceRole or nullptr if it has a different role.
     */
    LSubsurfaceRole *subsurface() const;

    /*!
     * @brief Constructor of the LSurface class.
     *
     * @param params Internal parameters of the library provided in the virtual constructor LCompositor::createSurfaceRequest().
     * @param textureUnit OpenGL texture unit.
     */
    LSurface(Params *params, GLuint textureUnit = 1);

    /*!
     * @brief Destructor of the LSurface class.
     *
     * Invoked after LCompositor::destroySurfaceRequest().
     */
    virtual ~LSurface();

    LSurface(const LSurface&) = delete;
    LSurface& operator= (const LSurface&) = delete;

    /*!
     * @brief Position given by the compositor.
     *
     * Position of the surface assigned with setPosC(), setXC() or setYC().
     */
    const LPoint &posC() const;

    /*!
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPosC(const LPoint &newPos);

    /*!
     * @brief Assigns the position.
     *
     * Assigns the position of the surface.
     */
    void setPosC(Int32 x, Int32 y);

    /*!
     * @brief Assigns the x component of the position.
     *
     * Assigns the x component of the position of the surface.
     */
    void setXC(Int32 x);

    /*!
     * @brief Assigns the y component of the position.
     *
     * Assigns the y component of the position of the surface.
     */
    void setYC(Int32 y);

    /*!
    * @brief Role position.
    *
    * Role position in compositor coordinates. If the surface has no role, the same value as posC() is returned.
    */
    const LPoint &rolePosC() const;

    /*!
     * @brief Surface size in buffer coordinates.
     */
    const LSize &sizeB() const;

    /*!
     * @brief Surface size in surface coordinates.
     */
    const LSize &sizeS() const;

    /*!
     * @brief Surface size in compositor coordinates.
     */
    const LSize &sizeC() const;

    /*!
     * @brief Input region in surface coordinates.
     *
     * Region of the surface that is capable of receiving input, in surface coordinates.
     */
    const LRegion &inputRegionS() const;

    /*!
     * @brief Input region in compositor coordinates.
     *
     * Region of the surface that is capable of receiving input, in compositor coordinates.
     */
    const LRegion &inputRegionC() const;

    /*!
     * @brief Opaque region in surface coordinates.
     */
    const LRegion &opaqueRegionS() const;

    /*!
     * @brief Opaque region in compositor coordinates.
     */
    const LRegion &opaqueRegionC() const;

    /*!
     * @brief Translucent region in surface coordinates.
     *
     * Translucent region in surface coordinates (inverted opaque region).
     */
    const LRegion &translucentRegionS() const;

    /*!
     * @brief Translucent region in compositor coordinates.
     *
     * Translucent region in compositor coordinates (inverted opaque region).
     */
    const LRegion &translucentRegionC() const;

    /*!
     * @brief Damaged region in buffer coordinates.
     */
    const LRegion &damagesB() const;

    /*!
     * @brief Damaged region in compositor coordinates.
     */
    const LRegion &damagesC() const;

    /*!
     * @brief Enters an output
     *
     * Notifies a surface that it has entered the visible area of an output.\n
     * The client uses this information to match the scale of the surface to that of the output.\n
     * You can access the list of outputs in which the surface is visible with outputs().
     *
     * @param output Output that the surface enters.
     */
    void sendOutputEnterEvent(LOutput *output);

    /*!
     * @brief Leaves an output
     *
     * Notifies a surface that it is no longer visible on an output.\n
     * You can access the list of outputs on which the surface is visible with outputs().
     * @param output Output on which the surface is no longer visible.
    */
    void sendOutputLeaveEvent(LOutput *output);

    /*!
     * @brief Surface intersected outputs
     *
     * List of outputs in which the surface is visible, modifiable with the sendOutputEnterEvent() and sendOutputLeaveEvent() methods.
     */
    const list<LOutput*>&outputs() const;

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
     * @returns true if the surface is minimized and false otherwise.
     */
    bool minimized() const;

    /*!
     * @brief Sets the minimized property
     *
     * Minimize or de-minimize the surface and all its children.
     */
    void setMinimized(bool state);

    /*!
     * @brief Input capability
     *
     * Indicates whether the surface is able to receive input.
     */
    bool receiveInput() const;

    /*!
     * @brief Buffer scale
     *
     * Scale of the surface buffer. You can listen for changes to this property with scaleChanged().
     */
    Int32 bufferScale() const;

    /*!
     * @brief OpenGL texture
     *
     * Representation of the surface's buffer as an OpenGL texture.\n
     *
     * @warning It could return nullptr if the surface is not currently mapped.
     */
    LTexture *texture() const;

    /*!
     * @brief Native buffer handle
     *
     * Handle to the Wayland buffer of the surface.
     */
    wl_buffer *buffer() const;

    /*!
     * @brief Presence of damages
     *
     * Indicates if the surface has new damages since the last time the requestNextFrame() method was called.\n
     * You can access the damage region with damagesB() or damagesC().
     */
    bool hasDamage() const;

    UInt32 damageId() const;

    /*!
     * @brief Returns the rendering callback
     *
     * Notifies the surface that it's time for it to draw its next frame.\n
     * If not called, the given surface will not update its content.
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
     * @returns A pointer to the parent surface or nullptr if it does not have a parent.
     */
    LSurface *parent() const;

    /*!
     * @brief Topmost parent of the surface.
     *
     * @returns A pointer to the topmost parent of the surface, or nullptr if it does not have a parent.
     */
    LSurface *topmostParent() const;

    /*!
     * @brief Child surfaces.
     *
     * List of child surfaces.
     */
    const list<LSurface*>&children() const;

    bool isPopupSubchild() const;
    bool hasPopupSubchild() const;
    bool isSubchildOf(LSurface *parent) const;

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
     * @brief Notifies when the surface is raised
     *
     * Reimplement this virtual method if you want to be notified when the surface is repositioned at the end of the compositor's list of surfaces.
     *
     * #### Default Implementation
     * @snippet LSurfaceDefault.cpp inputRegionChanged
     */
    virtual void raised();
    virtual void orderChanged();
    virtual void requestedRepaint();
    virtual void minimizedChanged();
/// @}

    LPRIVATE_IMP(LSurface)
};

#endif // LSURFACE_H
