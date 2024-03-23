#ifndef LSESSIONLOCKROLE_H
#define LSESSIONLOCKROLE_H

#include <LBaseSurfaceRole.h>
#include <queue>

class Louvre::LSessionLockRole : public LBaseSurfaceRole
{
public:

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

    LSessionLockRole(const void *params);

    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    virtual const LPoint &rolePos() const override;

private:
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
