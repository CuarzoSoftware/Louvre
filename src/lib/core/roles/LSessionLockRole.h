#ifndef LSESSIONLOCKROLE_H
#define LSESSIONLOCKROLE_H

#include <LBaseSurfaceRole.h>
#include <queue>

/**
 * @brief Surface displayed during a session lock.
 *
 * LSessionLockRole can be used by clients to display arbitrary graphics while the session is locked.\n
 * When a client requests to lock a session (see LSessionLockManager::lockRequest()), it first creates an LSessionLockRole
 * for each initialized output, with a size equal to each output's size (the size is then automatically adjusted if the outputs change).\n
 * All surfaces with this role are initially unmapped, they only become mapped if permission to lock the session is granted to the client.
 * Later, they become unmapped again when the session is unlocked.
 *
 * While the session is locked, only surfaces created by the locking client LSessionLockManager::client() should be rendered and receive input events.\n
 * If the client crashes before unlocking the session (see @ref LSessionLockManager::DeadLocked), the session must remain locked, and a fallback mechanism should
 * be used to authenticate the user, such as relaunching the locking client.
 */
class Louvre::LSessionLockRole : public LBaseSurfaceRole
{
public:

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LSessionLockRole;

    /**
     * @brief Constructor of the LSessionLockRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LSessionLockRole(const void *params) noexcept;

    LCLASS_NO_COPY(LSessionLockRole)

    /**
     * @brief Destructor of the LSessionLockRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LSessionLockRole() = default;

    /**
     * @brief The output the surface is assigned to.
     *
     * @warning It may return `nullptr` if the output is unplugged.
     */
    LOutput *exclusiveOutput() const override
    {
        return m_output;
    }

    /**
     * @brief Surface position.
     *
     * The default implementation returns the same position as the output it is assigned to.
     *
     * @par Default Implementation
     * @snippet LSessionLockRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

private:
    struct Params
    {
        LResource *resource;
        LSurface *surface;
        LOutput *output;
    };

    struct Configuration
    {
        LSize size;
        UInt32 serial { 0 };
    };

    friend class Louvre::Protocols::SessionLock::RSessionLockSurface;
    friend class Louvre::LCompositor;
    friend class Louvre::LOutput;
    void handleSurfaceCommit(CommitOrigin origin) override;
    void configure(const LSize &size) noexcept;
    void sendPendingConfiguration() noexcept;
    LWeak<LOutput> m_output;
    std::queue<Configuration> m_sentConfs;
    LSize m_currentSize { -1, -1};
    LSize m_pendingSize;
    UInt32 m_pendingSerial;
    bool m_hasPendingConf { false };
    bool m_isComplete { false };
};

#endif // LSESSIONLOCKROLE_H
