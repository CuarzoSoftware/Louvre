#ifndef LFOREIGNTOPLEVELCONTROLLER_H
#define LFOREIGNTOPLEVELCONTROLLER_H

#include <LFactoryObject.h>
#include <LRect.h>
#include <LWeak.h>

/**
 * @brief Foreign Toplevel Controller
 *
 * Clients using the [Wlr Foreign Toplevel Management](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1)
 * protocol can receive information about all open toplevel windows within the compositor and request to modify their state,
 * such as activating, minimizing, maximizing them, and more.
 *
 * It is typically used by dock/panel applications to display open windows within a taskbar and control their state.
 *
 * This class represents a single toplevel being controlled by a specific client using the protocol and provides information
 * such as the controller client(), the toplevelRole() it is controlling, and the taskbar() and taskbarIconRect() properties, which indicate
 * where the toplevelRole() is being represented (e.g., as a tab, icon, etc.) whithin one of the controller client surfaces.
 *
 * @note All LForeignToplevelController objects created for a specific toplevel window can be accessed from LToplevelRole::foreignControllers().
 *
 * @section Requests
 *
 * Clients using this protocol can trigger the following LToplevelRole class requests:
 *
 * - LToplevelRole::setMaximizedRequest() and LToplevelRole::unsetMaximizedRequest()
 * - LToplevelRole::setFullscreenRequest() and LToplevelRole::unsetFullscreenRequest()
 * - LToplevelRole::setMinimizedRequest() and LToplevelRole::unsetMinimizedRequest()
 * - LToplevelRole::activateRequest() and LToplevelRole::closeRequest()
 *
 * To distinguish who is triggering a request, LToplevelRole::requesterController() can be employed:
 *
 * - If LToplevelRole::requesterController() returns `nullptr` during a request, it means the client owner of the toplevel is triggering it.
 * - Otherwise, it means the request is triggered by the given LForeignToplevelController client returned by LToplevelRole::requesterController().
 *
 * Whenever properties of a toplevel, such as its state, title, appId, etc change, Louvre automatically notifies its LToplevelRole::foreignControllers().
 *
 * @section Security
 *
 * For security reasons, a compositor should only allow well-known clients to use this protocol. Check LCompositor::globalsFilter() to see how to accomplish that.
 *
 * @note To disable this protocol remove its global from LCompositor::createGlobalsRequest().
 *
 * There are also two ways to prevent specific toplevels from being controlled. One way is through LToplevelRole::foreignControllerFilter(),
 * and a second way is calling `finished()` for a specific global resource within LClient::foreignToplevelManagerGlobals(), which prevents the resource
 * from receiving information from newly created toplevels and from controlling them.
 */
class Louvre::LForeignToplevelController : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LForeignToplevelController;

    /**
     * @brief Constructor of the LForeignToplevelController class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LForeignToplevelController(const void *params) noexcept;

    LCLASS_NO_COPY(LForeignToplevelController)

    /**
     * @brief Destructor of the LForeignToplevelController class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LForeignToplevelController() noexcept;

    /**
     * @brief Client controlling the given toplevelRole().
     */
    LClient *client() const noexcept;

    /**
     * @brief Wayland resource
     *
     * @see [zwlr_foreign_toplevel_handle_v1](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1)
     */
    Protocols::ForeignToplevelManagement::RForeignToplevelHandle &resource() const noexcept
    {
        return m_resource;
    }

    /**
     * @brief Toplevel window being controlled.
     *
     * @return It can return `nullptr` if the toplevel has been destroyed.
     */
    LToplevelRole *toplevelRole() const noexcept;

    /**
     * @brief Taskbar surface.
     *
     * This property, along with taskbarIconRect(), indicates where the toplevelRole() is represented (e.g., as a tab, icon, etc.),
     * which can be utilized, for example, for minimizing animations.
     *
     * @see taskbarChanged()
     *
     * @return The surface where the toplevel is being represented, or `nullptr` if not set.
     */
    LSurface *taskbar() const noexcept
    {
        return m_taskbar;
    }

    /**
     * @brief Rectangle within the taskbar where the toplevel is represented.
     *
     * Defines a rectangle within taskbar(), in surface-local coordinates, where the toplevel is being represented.
     *
     * @see taskbarChanged()
     *
     * @warning This value is meaningless if taskbar() is `nullptr`.
     */
    const LRect &taskbarIconRect() const noexcept
    {
        return m_taskbarIconRect;
    }

    /**
     * @brief Notifies that taskbar() and/or taskbarIconRect() changed.
     *
     * #### Default Implementation
     * @snippet LForeignToplevelControllerDefault.cpp taskbarChanged
     */
    virtual void taskbarChanged();

private:
    friend class Protocols::ForeignToplevelManagement::RForeignToplevelHandle;
    Protocols::ForeignToplevelManagement::RForeignToplevelHandle &m_resource;
    LRect m_taskbarIconRect;
    LWeak<LSurface> m_taskbar;
};

#endif // LFOREIGNTOPLEVELCONTROLLER_H
