#ifndef LBASESURFACEROLE_H
#define LBASESURFACEROLE_H

#include <LNamespaces.h>
#include <LPoint.h>

 /*!
  * @brief Base class for surface roles.
  *
  * The LBaseSurfaceRole class provides a base interface for creating surface roles that are compatible with the library.\n
  * It allows for implementing the logic of positioning the surface that will acquire the role, assigning the role ID,
  * handling requests from the [wl_surface](https://wayland.app/protocols/wayland#wl_surface) interface, among other functionalities.
  *
  * @warning Only use this class if you wish to create additional surface roles beyond those offered by the library.
  */
class Louvre::LBaseSurfaceRole
{
public:

    /*!
     * @brief Constructor of LBaseSurfaceRole class.
     *
     * Once initialization is complete, the surface provided in the second argument acquires the role and the virtual method LSurface::roleChanged()
     * is invoked. The role can later be accessed with the LSurface::role() method.
     *
     * @param resource Resource granted by the role's protocol interface. If the role does not have a resource, the one accessible with LSurface::resource() should be passed.
     * @param surface Surface that will acquire the role.
     * @param roleId ID of the role, later accessible with the LSurface::roleId() method. Must be a value greater than 1000.
     *
     * @warning The ID range [0,1000] is reserved for roles offered by the library.
     */
    LBaseSurfaceRole(wl_resource *resource, LSurface *surface, UInt32 roleId);

    /*!
     * @brief The LBaseSurfaceRole class destructor.
     */
    virtual ~LBaseSurfaceRole();

    LBaseSurfaceRole(const LBaseSurfaceRole&) = delete;
    LBaseSurfaceRole& operator= (const LBaseSurfaceRole&) = delete;

    /*!
     * @brief Position of the surface given its role.
     *
     * This method must return the **m_rolePosC** variable. It should be overridden to return the position of the surface according to the
     * logic given by the role protocol.\n
     * The LSurface class returns the position given by this method when calling LSurface::rolePosC().
     *
     * See the default implementation of this method in the roles offered by the library for more information.
     */
    virtual const LPoint &rolePosC() const = 0;

    /*!
     * @brief Role ID.
     *
     * Returns the ID of the role given in the constructor argument. The LSurface class returns this value with the LSurface::roleId() method.
     */
    UInt32 roleId();

    /*!
     * @brief Returns the compositor instance.
     */
    LCompositor *compositor() const;

    /*!
     * @brief Returns the surface that has acquired the role provided in the constructor.
     */
    LSurface *surface() const;

    /*!
     * @brief Returns the seat instance.
     */
    LSeat *seat() const;

    /*!
     * @brief Returns the Wayland resource for this role given in the constructor.
     */
    wl_resource *resource() const;

    class LBaseSurfaceRolePrivate;

    /*!
     * @brief Access to the private API of LBaseSurfaceRole.
     *
     * Returns an instance of the LBaseSurfaceRolePrivate class (following the ***PImpl Idiom*** pattern) which contains all the private members of LBaseSurfaceRole.\n
     * Used internally by the library.
     */
    LBaseSurfaceRolePrivate *baseImp() const;

protected:

    friend class Louvre::Globals::Surface;
    friend class Louvre::LSurface;

    /*!
     * @brief Variable that stores the position of the role.
     *
     * Variable to store the position of the surface according to its role. It must be assigned and returned in the implementation of the LBaseSurfaceRole::rolePosC() method.
     */
    mutable LPoint m_rolePosC;

    /*!
     * @brief Notifies a change of the global scale of the compositor.
     *
     * Called when the compositor changes its global scale. It should be used to update role variables that use compositor coordinates.
     */
    virtual void globalScaleChanged(Int32 oldScale, Int32 newScale);

    /*!
     * @brief Asks if the surface commit should be processed.
     *
     * The commit is processed only if **true** is returned.
     *
     * @param origin Origin of the request. In some protocols the commit is called by other surfaces. For example, surfaces the ***subsurface*** role only accept commits from their parent
     * if they are in synchronous mode.
     */
    virtual bool acceptCommitRequest(Globals::CommitOrigin origin);

    /*!
     * @brief Notifies a surface commit.
     *
     * Access to the **wl_surface::commit** request of the surface. It should be used by protocols that require atomic changes via commits.
     */
    virtual void handleSurfaceCommit();

    /*!
     * @brief Notifies a new surface buffer attachment.
     *
     * Access to the **wl_surface::attach** request of the surface.
     */
    virtual void handleSurfaceBufferAttach(wl_resource *buffer, Int32 x, Int32 y);

    /*!
     * @brief Notifies a surface buffer offset change.
     *
     * Provides access to the offset generated by the **wl_surface::attach** and **wl_surface::offset** requests.
     */
    virtual void handleSurfaceOffset(Int32 x, Int32 y);

    /*!
     * @brief Notifies a parent surface commit.
     *
     * Access to the **wl_surface::commit** request of the parent surface.
     */
    virtual void handleParentCommit();

    /*!
     * @brief Notifies when the mapping state of the parent surface changes.
     */
    virtual void handleParentMappingChange();

    /*!
     * @brief Notifies when the parent surface changes.
     */
    virtual void handleParentChange();

private:
    LBaseSurfaceRolePrivate *m_baseImp = nullptr;
};

#endif // LBASESURFACEROLE_H
