#ifndef LBASESURFACEROLE_H
#define LBASESURFACEROLE_H

#include <LFactoryObject.h>
#include <LWeak.h>
#include <LPoint.h>

 /**
  * @brief Base class for surface roles.
  * @ingroup roles
  *
  * A role defines the type and functionality of a surface. Some commonly known roles are LToplevelRole and LPopupRole.
  *
  * @note This class is primarily for library maintainers or individuals who wish to implement additional roles beyond those provided by the library.
  *
  * This class is responsible, among other things, for:
  *
  * @li Creating the surface role (returned in LSurface::role()).
  * @li Assigning the role's unique ID (returned in LSurface::roleId()).
  * @li Assigning the surface position according to the role's protocol (returned in LSurface::rolePos()).
  * @li Providing access to native requests of the [wl_surface](https://wayland.app/protocols/wayland#wl_surface) interface.
  *
  * @section role_creation Creating a Custom Role
  *
  * The steps for creating a custom role are as follows:
  *
  * @li Implement the native protocol interfaces of the role protocol.
  * @li Create an LResource wrapper for the role's `wl_resource`, or use LSurface::surfaceResource() if the role does not have its own `wl_resource`.
  * @li Create a subclass of LBaseSurfaceRole.
  * @li Select a unique role ID.
  * @li Override the LBaseSurfaceRole::rolePos() method and implement the positioning logic of the role.
  * @li Override any other protected methods in LBaseSurfaceRole to handle native requests of the `wl_surface` interface.
  *
  * To assign the custom role to a surface at runtime, follow these steps:
  *
  * @li Create an instance of your custom LBaseSurfaceRole.
  * @li Call the `setPendingRole()` method (private) on the target surface, passing the custom role instance.
  * @li Call the `applyPendingRole()` method (private) to apply the pending role change. This will trigger the LSurface::roleChanged() event automatically.
  *
  * By following these steps, you can create a custom role and assign it to a surface dynamically during runtime.
  */
class Louvre::LBaseSurfaceRole : public LFactoryObject
{
public:

    /**
     * @brief Commit origin
     */
    enum CommitOrigin
    {
        /// @brief The commit is requested by the surface itself
        Itself,

        /// @brief The commit is requested by the parent surface
        Parent
    };

    /**
     * @brief Constructor of LBaseSurfaceRole class.
     *
     * @param resource Resource granted by the role's protocol interface. If the role does not have a resource, the one accessible with LSurface::surfaceResource() should be passed.
     * @param surface Surface that will acquire the role.
     * @param roleId ID of the role, later accessible with the LSurface::roleId() method. Must be a value greater than 1000.
     *
     * @note The ID range [0,1000] is reserved for roles offered by the library.
     */
    LBaseSurfaceRole(LFactoryObject::Type type, LResource *resource, LSurface *surface, UInt32 roleId) noexcept;

    /**
     * @brief The LBaseSurfaceRole class destructor.
     *
     * @warning The `surface()` handle always remains valid during the destructor call.
     *          However, `LSurface::role()` returns `nullptr` because `LSurface::roleChanged()`
     *          is notified beforehand and requires the role to be valid.
     */
    ~LBaseSurfaceRole();

    LCLASS_NO_COPY(LBaseSurfaceRole)

    /**
     * @brief Position of the surface given its role.
     *
     * This method must return the **m_rolePos** variable. It should be overridden to return the position of the surface according to the
     * logic given by the role protocol.\n
     * The LSurface class returns the position given by this method when calling LSurface::rolePos().
     *
     * See the default implementation of this method in the roles offered by the library for more information.
     */
    virtual const LPoint &rolePos() const = 0;

    /**
     * @brief The output to which the surface should be constrained.
     *
     * When this property is set, the surface should be displayed only on the specified output.\n
     * Surfaces with the LSessionLockRole or LLayerRole explicitly indicate the output on which
     * the surface should be displayed. This property is also useful for LToplevelRole surfaces
     * when they are maximized or in fullscreen mode.
     *
     * @note This is a hint, you may choose to ignore it.
     *
     * @return The output the surface should be constrained to, or `nullptr` if not constrained to any output.
     */
    virtual LOutput *exclusiveOutput() const
    {
        return nullptr;
    }

    /**
     * @brief Role ID.
     *
     * Returns the ID of the role given in the constructor argument. The LSurface class returns this value with the LSurface::roleId() method.
     */
    UInt32 roleId() const noexcept
    {
        return m_roleId;
    }

    /**
     * @brief Returns the surface that has acquired the role provided in the constructor.
     */
    LSurface *surface() const noexcept
    {
        return m_surface;
    }

    /**
     * @brief Returns the Wayland resource for this role given in the constructor.
     */
    LResource *resource() const
    {
        return m_resource;
    }

    /**
     * @brief Client owner of the surface role.
     */
    LClient *client() const noexcept;

protected:
    friend class Protocols::Wayland::RSurface;
    friend class Louvre::LSurface;

    /**
     * @brief Variable that stores the surface position given the role.
     *
     * Variable to store the position of the surface according to its role. It must be assigned and returned in the implementation of the LBaseSurfaceRole::rolePos() method.
     */
    mutable LPoint m_rolePos;

    /**
     * @brief Asks if the surface commit should be processed.
     *
     * The commit is processed only if **true** is returned.
     *
     * @param origin Origin of the request. In some protocols the commit is called by other surfaces. For example, surfaces with the ***subsurface*** role only accept commits from their parent
     * if they are in synchronous mode.
     */
    virtual bool acceptCommitRequest(CommitOrigin origin);

    /**
     * @brief Notifies a surface commit.
     *
     * Access to the **wl_surface::commit** request of the surface. It should be used by protocols that require atomic changes via commits.
     */
    virtual void handleSurfaceCommit(CommitOrigin origin);

    /**
     * @brief Notifies a new surface buffer attachment.
     *
     * Access to the **wl_surface::attach** request of the surface.
     *
     * @param buffer The Wayland buffer resource.
     * @param x X-coordinate offset.
     * @param y Y-coordinate offset.
     */
    virtual void handleSurfaceBufferAttach(wl_resource *buffer, Int32 x, Int32 y);

    /**
     * @brief Notifies a surface buffer offset change.
     *
     * Provides access to the offset generated by the **wl_surface::attach** and **wl_surface::offset** requests.
     */
    virtual void handleSurfaceOffset(Int32 x, Int32 y);

    /**
     * @brief Notifies a parent surface commit.
     *
     * Access to the **wl_surface::commit** request of the parent surface.
     */
    virtual void handleParentCommit();

    /**
     * @brief Notifies when the mapping state of the parent surface changes.
     */
    virtual void handleParentMappingChange();

    /**
     * @brief Notifies when the parent surface changes.
     */
    virtual void handleParentChange();

    void validateDestructor() noexcept;
private:
    LWeak<LSurface> m_surface;
    LWeak<LResource> m_resource;
    UInt32 m_roleId;
    bool m_destructorValidated { false };
};

#endif // LBASESURFACEROLE_H
