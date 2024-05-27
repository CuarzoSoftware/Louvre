#ifndef LLAYERROLE_H
#define LLAYERROLE_H

#include <LBaseSurfaceRole.h>
#include <LExclusiveZone.h>
#include <LBitset.h>
#include <LOutput.h>
#include <list>

class Louvre::LLayerRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LLayerRole;

    enum KeyboardInteractivity
    {
        NoInteractivity = 0,
        Exclusive = 1,
        OnDemand = 2
    };

    enum AtomsChanges
    {
        SizeChanged                     = static_cast<UInt32>(1) << 0,
        AnchorChanged                   = static_cast<UInt32>(1) << 1,
        ExclusiveZoneChanged            = static_cast<UInt32>(1) << 2,
        MarginChanged                   = static_cast<UInt32>(1) << 3,
        KeyboardInteractivityChanged    = static_cast<UInt32>(1) << 4,
        LayerChanged                    = static_cast<UInt32>(1) << 5,
        ExclusiveEdgeChanged            = static_cast<UInt32>(1) << 6,
    };

    struct Atoms
    {
        LSize size;
        LBitset<LEdge> anchor;
        Int32 exclusiveZone;
        LMargins margin;
        KeyboardInteractivity keyboardInteractivity { NoInteractivity };
        LEdge exclusiveEdge { LEdgeNone };
        LSurfaceLayer layer;
    };

    struct Configuration
    {
        LSize size;
        UInt32 serial;
    };

    LLayerRole(const void *params) noexcept;

    LCLASS_NO_COPY(LLayerRole)

    ~LLayerRole() noexcept;

    virtual const LPoint &rolePos() const override;
    virtual void configureRequest();
    virtual void atomsChanged(LBitset<AtomsChanges> changes, const Atoms &prevAtoms);
    void configureSize(const LSize &size) noexcept;
    void configureSize(Int32 width, Int32 height) noexcept;

    const Configuration &currentConf() const noexcept
    {
        return m_lastACKConf;
    }

    const Configuration &pendingConf() const noexcept
    {
        return m_pendingConf;
    }

    const Atoms &atoms() const noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    const LSize &size() const noexcept
    {
        return atoms().size;
    }

    LBitset<LEdge> anchor() const noexcept
    {
        return atoms().anchor;
    }

    const LExclusiveZone &exclusiveZone() const noexcept
    {
        return m_exclusiveZone;
    }

    Int32 exclusiveZoneSize() const noexcept
    {
        return atoms().exclusiveZone;
    }

    const LMargins &margin() const noexcept
    {
        return atoms().margin;
    }

    KeyboardInteractivity keyboardInteractivity() const noexcept
    {
        return atoms().keyboardInteractivity;
    }

    LEdge exclusiveEdge() const noexcept
    {
        return atoms().exclusiveEdge;
    }

    LSurfaceLayer layer() const noexcept
    {
        return atoms().layer;
    }

    LOutput *exclusiveOutput() const override
    {
        return m_exclusiveZone.output();
    }

    void setExclusiveOutput(LOutput *output) noexcept
    {
        m_exclusiveZone.setOutput(output);
    }

    const std::string &nameSpace() noexcept
    {
        return m_namespace;
    }

    void close() noexcept;

private:
    friend class Louvre::Protocols::LayerShell::RLayerSurface;

    enum Flags
    {
        HasPendingSize                  = static_cast<UInt32>(1) << 0,
        HasPendingAnchor                = static_cast<UInt32>(1) << 1,
        HasPendingExclusiveZone         = static_cast<UInt32>(1) << 2,
        HasPendingMargin                = static_cast<UInt32>(1) << 3,
        HasPendingKeyboardInteractivity = static_cast<UInt32>(1) << 4,
        HasPendingLayer                 = static_cast<UInt32>(1) << 5,
        HasPendingExclusiveEdge         = static_cast<UInt32>(1) << 6,
        HasPendingInitialConf           = static_cast<UInt32>(1) << 7,
        HasConfToSend                   = static_cast<UInt32>(1) << 8,
        ClosedSent                      = static_cast<UInt32>(1) << 9,
    };

    Atoms &currentAtoms() noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    Atoms &pendingAtoms() noexcept
    {
        return m_atoms[1 - m_currentAtomsIndex];
    }

    LEdge edgesToSingleEdge() const noexcept;

    void handleSurfaceCommit(CommitOrigin origin) noexcept override;

    LExclusiveZone m_exclusiveZone { LEdgeNone, 0 };
    LBitset<Flags> m_flags { HasPendingInitialConf };
    Atoms m_atoms[2];
    UInt8 m_currentAtomsIndex { 0 };
    Configuration m_pendingConf, m_lastACKConf;
    std::list<Configuration> m_sentConfs;
    std::string m_namespace;
};

#endif // LLAYERROLE_H
