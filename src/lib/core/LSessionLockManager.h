#ifndef LSESSIONLOCKMANAGER_H
#define LSESSIONLOCKMANAGER_H

#include <LFactoryObject.h>
#include <LWeak.h>

/**
 * @brief Manages session lock requests and state changes.
 *
 * Clients using the [Session Lock](https://wayland.app/protocols/ext-session-lock-v1) protocol can request the compositor to lock the user session and display
 * arbitrary graphics (see LSessionLockRole) such as an authentication form to allow the user to unlock the session.\n
 * This class allows you to accept/decline such requests (see lockRequest()) and monitor changes in the session state (stateChanged()).
 */
class Louvre::LSessionLockManager : public LFactoryObject
{
public:

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LSessionLockManager;

    /**
     * @brief Represents the state of the session.
     */
    enum State
    {
        Unlocked,   /**< The session is unlocked. */
        Locked,     /**< The session is locked. */
        DeadLocked  /**< The session is locked, but the locking client died. */
    };

    /**
     * @brief LSessionLockManager class constructor.
     *
     * There is only one instance of LSessionLockManager, which can be accessed from LCompositor::sessionLockManager().
     *
     * @param params Internal library parameters provided in the virtual LCompositor::createSessionLockManagerRequest() constructor.
     */
    LSessionLockManager(const void *params) noexcept;

    /**
     * @brief Gets the client locking the session.
     *
     * @return A pointer to the client locking the session, or `nullptr` if the session is unlocked or the locking client died.
     */
    LClient *client() const noexcept;

    /**
     * @brief Gets the Session Lock Surface Roles.
     *
     * The client creates one for each output.
     */
    const std::vector<LSessionLockRole*> &roles() const noexcept;

    /**
     * @brief Gets the current state of the session.
     */
    State state() const noexcept
    {
        if (m_state == Locked)
            return client() == nullptr ? DeadLocked : Locked;
        return Unlocked;
    }

    /**
     * @brief Forces the session to unlock.
     *
     * If the locking client dies without unlocking the session, the session remains locked (@ref DeadLocked state).
     * This method allows the compositor to forcibly unlock the session, ideally after a fallback user authentication mechanism.
     */
    void forceUnlock();

    /**
     * @brief Handles a lock request from a client.
     *
     * This method determines whether to grant permission for the client to lock the session.
     * Returning `true` indicates permission, while `false` denies the request.
     *
     * @warning It's recommended that the client be a trusted and well-known entity.
     *
     * This method is only invoked when the session state is @ref Unlocked or @ref DeadLocked.
     * Louvre automatically ignores requests in the @ref Locked state.
     *
     * @param client The client requesting to lock the session.
     *
     * #### Default Implementation
     * @snippet LSessionLockManagerDefault.cpp lockRequest
     */
    virtual bool lockRequest(LClient *client);

    /**
     * @brief Notifies a change in the session state.
     *
     * #### Default Implementation
     * @snippet LSessionLockManagerDefault.cpp stateChanged
     */
    virtual void stateChanged();

    /// @cond OMIT
private:
    friend class Protocols::SessionLock::RSessionLock;
    friend class LSessionLockRole;
    friend class LOutput;
    LWeak<Protocols::SessionLock::RSessionLock> m_sessionLockRes;
    State m_state { Unlocked };
    std::vector<LSessionLockRole*> m_dummy;
    /// @endcond
};

#endif // LSESSIONLOCKMANAGER_H
