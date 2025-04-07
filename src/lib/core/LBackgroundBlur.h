#ifndef LBACKGROUNDBLUR_H
#define LBACKGROUNDBLUR_H

#include <LFactoryObject.h>
#include <LBitset.h>
#include <LWeak.h>
#include <list>
#include <memory>
#include <string>

class Louvre::LBackgroundBlur : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LBackgroundBlur;

    enum State
    {
        Disabled,
        Enabled
    };

    enum Style
    {
        Dark,
        Light
    };

    enum AtomChanges : UInt8
    {
        StateChanged    = static_cast<UInt8>(1) << 0,
        StyleChanged    = static_cast<UInt8>(1) << 1,
        RegionChanged   = static_cast<UInt8>(1) << 2,
        SerialChanged   = static_cast<UInt8>(1) << 3,
    };

    struct Configuration
    {
        State state { Disabled };
        Style style { Light };
        UInt32 serial { 0 };
    };

    struct Atoms
    {
        State state { Disabled };
        Style style { Light };
        std::unique_ptr<const LRegion> region;
        std::unique_ptr<const std::string> path;
        UInt32 serial { 0 };
    };

    LBackgroundBlur(const void *params) noexcept;
    LCLASS_NO_COPY(LBackgroundBlur)
    ~LBackgroundBlur() noexcept;

    Protocols::BackgroundBlur::RBackgroundBlur *backgroundBlurResource() const noexcept;

    bool supported() const noexcept { return backgroundBlurResource() != nullptr; };
    const Configuration &pendingConfiguration() const noexcept { return m_pendingConfiguration; };
    const Atoms &atoms() const noexcept { return m_atoms[m_currentAtomsIndex]; };

    State state() const noexcept { return atoms().state; };
    Style style() const noexcept { return atoms().style; };
    const LRegion *region() const noexcept { return atoms().region.get(); };
    const std::string *path() const noexcept { return atoms().path.get(); };
    UInt32 serial() const noexcept { return atoms().serial; };

    bool visible() const noexcept { return state() && (region() || path()); };

    void configureState(State state) noexcept
    {
        if (!supported())
            return;
        updateSerial();
        m_flags.add(HasStateToSend);
        m_pendingConfiguration.state = state;
    }
    void configureStyle(Style style) noexcept
    {
        if (!supported())
            return;
        updateSerial();
        m_flags.add(HasStyleToSend);
        m_pendingConfiguration.style = style;
    }

    virtual void configureRequest();
    virtual void atomsChanged(LBitset<AtomChanges> changes, const Atoms &prevAtoms);

    LSurface *surface() const noexcept { return &m_surface; };
private:
    friend class LCompositor;
    friend class Protocols::BackgroundBlur::RBackgroundBlur;
    friend class Protocols::Wayland::RSurface;

    enum Flags : UInt8
    {
        HasStateToSend = static_cast<UInt32>(1) << 0,
        HasStyleToSend = static_cast<UInt32>(1) << 1,
        AssignedRegion = static_cast<UInt32>(1) << 2,
        Destroyed      = static_cast<UInt32>(1) << 3,
    };

    Atoms &currentAtoms() noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    Atoms &pendingAtoms() noexcept
    {
        return m_atoms[1 - m_currentAtomsIndex];
    }

    void handleCommit() noexcept;
    void fullAtomsUpdate();
    void sendPendingConfiguration() noexcept;
    void updateSerial() noexcept;
    void reset() noexcept;

    LSurface &m_surface;
    LWeak<Protocols::BackgroundBlur::RBackgroundBlur> m_backgroundBlurRes;
    LBitset<Flags> m_flags;
    Atoms m_atoms[2];
    UInt8 m_currentAtomsIndex { 0 };

    Configuration m_pendingConfiguration;
    std::list<Configuration> m_sentConfigurations;
};

#endif // LBACKGROUNDBLUR_H
