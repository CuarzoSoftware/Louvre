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

    enum AtomicPropChanges
    {
        SizeChanged                     = static_cast<UInt32>(1) << 0,
        AnchorChanged                   = static_cast<UInt32>(1) << 1,
        ExclusiveZoneChanged            = static_cast<UInt32>(1) << 2,
        MarginChanged                   = static_cast<UInt32>(1) << 3,
        KeyboardInteractivityChanged    = static_cast<UInt32>(1) << 4,
        LayerChanged                    = static_cast<UInt32>(1) << 5,
        ExclusiveEdgeChanged            = static_cast<UInt32>(1) << 6,
    };

    struct AtomicProps
    {
        LSize size;
        LBitset<LEdge> anchor;
        Int32 exclusiveZone;
        LMargin margin;
        KeyboardInteractivity keyboardInteractivity;
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
    virtual void atomicPropsChanged(LBitset<AtomicPropChanges> changes, const AtomicProps &prevAtomicProps);
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

    const AtomicProps &atomicProps() const noexcept
    {
        return m_atomicProps[m_currentAtomicPropsIndex];
    }

    const LSize &size() const noexcept
    {
        return atomicProps().size;
    }

    LBitset<LEdge> anchor() const noexcept
    {
        return atomicProps().anchor;
    }

    const LExclusiveZone &exclusiveZone() const noexcept
    {
        return m_exclusiveZone;
    }

    Int32 exclusiveZoneSize() const noexcept
    {
        return atomicProps().exclusiveZone;
    }

    const LMargin &margin() const noexcept
    {
        return atomicProps().margin;
    }

    KeyboardInteractivity keyboardInteractivity() const noexcept
    {
        return atomicProps().keyboardInteractivity;
    }

    LEdge exclusiveEdge() const noexcept
    {
        return atomicProps().exclusiveEdge;
    }

    LSurfaceLayer layer() const noexcept
    {
        return atomicProps().layer;
    }

    LOutput *output() const noexcept
    {
        return m_exclusiveZone.output();
    }

    void setOutput(LOutput *output) noexcept
    {
        m_exclusiveZone.setOutput(output);
    }

    const std::string &nameSpace() noexcept
    {
        return m_namespace;
    }

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
    };

    AtomicProps &currentProps() noexcept
    {
        return m_atomicProps[m_currentAtomicPropsIndex];
    }

    AtomicProps &pendingProps() noexcept
    {
        return m_atomicProps[1 - m_currentAtomicPropsIndex];
    }

    LEdge edgesToSingleEdge() const noexcept;

    void handleSurfaceCommit(CommitOrigin origin) noexcept override;

    LExclusiveZone m_exclusiveZone { LEdgeNone, 0, this };
    LBitset<Flags> m_flags { HasPendingInitialConf };
    AtomicProps m_atomicProps[2];
    UInt8 m_currentAtomicPropsIndex { 0 };
    Configuration m_pendingConf, m_lastACKConf;
    std::list<Configuration> m_sentConfs;
    std::string m_namespace;
};

#endif // LLAYERROLE_H
