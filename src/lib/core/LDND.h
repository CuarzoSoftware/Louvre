#ifndef LDNDMANAGER_H
#define LDNDMANAGER_H

#include <LObject.h>
#include <LPointerButtonEvent.h>

/**
 * @brief Class for handling drag & drop sessions
 *
 * The LDND class provides control over drag & drop sessions and its unique instance can be accessed with LSeat::dnd().\n
 * It has virtual methods that notify when a client wants to start or cancels a drag & drop session, methods to
 * "drop" or cancel a data offering, and more.
 */
class Louvre::LDND : public LObject
{
public:
    struct Params;

    // TODO: Add doc
    void setFocus(LSurface *surface, const LPointF &localPos) noexcept;
    void sendMoveEvent(const LPointF &localPos, UInt32 ms) noexcept;
    const LEvent &triggeringEvent() const noexcept;

    /**
     * @brief Constructor of the LDNDManager class.
     *
     * There is a single instance of LDNDManager, which can be accessed from LSeat::dndManager().
     *
     * @param params Internal library parameters passed in the LCompositor::createDNDManagerRequest() virtual constructor.
     */
    inline LDND(const void *params) noexcept { L_UNUSED(params) }

    /**
     * @brief Destructor of the LDNDManager class.
     *
     * Invoked internally by the library after LCompositor::destroyDNDManagerRequest() is called.
     */
    virtual ~LDND() noexcept {}

    /// @cond OMIT
    LDND(const LDND&) = delete;
    LDND& operator= (const LDND&) = delete;
    /// @endcond

    /**
     * @brief Action flags for drag & drop sessions.
     *
     * Clients who start drag & drop sessions or receive a data offer notify which actions they support.\n
     * For example, when dragging a file from a file manager window to another, the desired action might be to
     * move or copy the file.\n
     * The library by default performs a match of actions supported by the source and destination client, giving preference
     * to the first one listed in the enum, except for LDNDManager::NoAction.\n
     * You can also select a different preferred action using the LDNDManager::setPreferredAction() method as exemplified in the
     * default implementation of LKeyboard::keyEvent().
     */
    enum Action : UInt32
    {
        /// No preferred action
        NoAction = 0,

        /// The preferred action is to copy
        Copy = 1,

        /// The preferred action is to move
        Move = 2,

        /// The destination client asks the source client the preferred action
        Ask = 4
    };

    /**
     * @brief Drag & drop session icon.
     *
     * LDNDIconRole of the surface used as drag & drop icon.
     *
     * @note Not all drag & drop sessions use an icon.
     *
     * @returns `nullptr` if there is no session going on, or if the source client did not assign an icon.
     */
    LDNDIconRole *icon() const noexcept;

    /**
     * @brief Surface that originates the drag & drop session.
     *
     * Surface from which the drag & drop session originates.
     *
     * @returns `nullptr` if there is no session going on.
     */
    LSurface *origin() const noexcept;

    /**
     * @brief Focused surface.
     *
     * Surface to which the data offer is being presented to.\n
     * It typically is the surface located under the cursor.
     *
     * @returns `nullptr` if there is no session going on or if no surface has focus.
     */
    LSurface *focus() const noexcept;

    /**
     * @brief Check if a drag & drop session is currently in progress.
     *
     * @return `true` if there is an ongoing drag & drop session, `false` otherwise.
     */
    bool dragging() const noexcept;

    /**
     * @brief Cancels the session.
     *
     * Cancels the current drag & drop session.\n
     * If there is no session going on, calling this method is a no-op.
     */
    void cancel() noexcept;

    /**
     * @brief Drop the data offer.
     *
     * Drop the data offer on the surface with active focus and ends the session.\n
     * The destination client then exchanges the information with the source client, using the specified action.\n
     * If there is no focused surface, the session is cancelled.
     *
     * The library invokes this method when the left mouse button is released (check the default implementation of LPointer::pointerButtonEvent()).
     */
    void drop() noexcept;

    /**
     * @brief Preferred action of the compositor.
     *
     * Returns the preferred actions of the compositor set whith setPreferredAction().
     */
    Action preferredAction() const noexcept;

    /**
     * @brief Assigns the preferred action.
     *
     * Assigns the compositor's preferred action for the session.
     * The library by default assigns the LDNDManager::Move action when holding down the `Shift`
     * key and the LDNDManager::Copy action when holding down the `Ctrl` key
     * (check the default implementation of LKeyboard::keyEvent()).\n
     * If the specified action is not supported by the source and destination client calling this method is a no-op.
     */
    void setPreferredAction(Action action) noexcept;

    /**
     * @brief Request to start a drag & drop session
     *
     * Reimplement this virtual method if you want to be notified when a client wants to start a drag & drop session.\n
     * This method is invoked only when there is no session in progress.\n
     * The default implementation validates that the client that originates the request has a surface with keyboard or pointer focus. If
     * neither condition is met the session is cancelled.
     *
     * #### Default implementation
     * @snippet LDNDManagerDefault.cpp startDragRequest
     */
    virtual void startDragRequest() noexcept;

    /**
     * @brief Notifies the cancellation of a session
     *
     * Reimplement this virtual method if you want to be notified when a drag & drop session is cancelled.\n
     * The default implementation repaints outputs where the drag & drop icon was visible.
     *
     * #### Default implementation
     * @snippet LDNDManagerDefault.cpp cancelled
     */
    virtual void cancelled() noexcept;

private:
    void sendLeaveEvent(LSurface *surface) noexcept;
    std::unique_ptr<LEvent> m_triggeringEvent { std::make_unique<LPointerButtonEvent>() };
    std::shared_ptr<LDNDSession> m_session;
    UInt32 m_compositorAction { NoAction };
};

#endif // LDNDMANAGER_H
