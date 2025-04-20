#ifndef LBACKGROUNDBLUR_H
#define LBACKGROUNDBLUR_H

#include <LRegion.h>
#include <LRRect.h>
#include <LFactoryObject.h>
#include <LBitset.h>
#include <LWeak.h>
#include <list>
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

    enum AreaType
    {
        Region,
        RoundRect,
        SVGPath
    };

    struct Area
    {
        LRegion region;
        LRRect roundRect;
        std::string svgPath;
    };

    enum PropChanges : UInt8
    {
        StateChanged          = static_cast<UInt8>(1) << 0,
        StyleChanged          = static_cast<UInt8>(1) << 1,
        AreaChanged           = static_cast<UInt8>(1) << 2,
        SerialChanged         = static_cast<UInt8>(1) << 3,
    };

    struct Configuration
    {
        State state { Disabled };
        Style style { Light };
        UInt32 serial { 0 };
    };

    struct Props
    {
        State state { Disabled };
        Style style { Light };
        AreaType areaType { Region };
        Area area { LRegion() };
        UInt32 serial { 0 };
        bool isEmpty { true };
        bool isFullSize { false };
    };

    LBackgroundBlur(const void *params) noexcept;
    LCLASS_NO_COPY(LBackgroundBlur)
    ~LBackgroundBlur() noexcept;

    Protocols::BackgroundBlur::RBackgroundBlur *backgroundBlurResource() const noexcept;

    bool supported() const noexcept { return backgroundBlurResource() != nullptr; };
    const Configuration &pendingConfiguration() const noexcept { return m_pendingConfiguration; };
    const Props &props() const noexcept { return m_props[m_currentPropsIndex]; };

    State state() const noexcept { return props().state; };
    Style style() const noexcept { return props().style; };
    AreaType areaType() const noexcept { return props().areaType; };
    const Area &area() const noexcept { return props().area; };
    UInt32 serial() const noexcept { return props().serial; };
    bool isEmpty() const noexcept { return props().isEmpty; };
    bool isFullSize() const noexcept { return props().isFullSize; };
    bool visible() const noexcept { return state() && !isEmpty(); };

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
    virtual void propsChanged(LBitset<PropChanges> changes, const Props &prevProps);

    LSurface *surface() const noexcept { return &m_surface; };
private:
    friend class LCompositor;
    friend class Protocols::BackgroundBlur::RBackgroundBlur;
    friend class Protocols::Wayland::RSurface;

    enum Flags : UInt8
    {
        HasStateToSend       = static_cast<UInt8>(1) << 0,
        HasStyleToSend       = static_cast<UInt8>(1) << 1,
        AssignedArea         = static_cast<UInt8>(1) << 2,
        Destroyed            = static_cast<UInt8>(1) << 3,
    };

    Props &currentProps() noexcept
    {
        return m_props[m_currentPropsIndex];
    }

    Props &pendingProps() noexcept
    {
        return m_props[1 - m_currentPropsIndex];
    }

    void handleCommit(bool sizeChanged) noexcept;
    void fullPropsUpdate(bool sizeChanged) noexcept;
    void sendPendingConfiguration() noexcept;
    void updateSerial() noexcept;
    void reset() noexcept;

    LSurface &m_surface;
    LWeak<Protocols::BackgroundBlur::RBackgroundBlur> m_backgroundBlurRes;
    LBitset<Flags> m_flags;
    Props m_props[2];
    UInt8 m_currentPropsIndex { 0 };
    Configuration m_pendingConfiguration;
    std::list<Configuration> m_sentConfigurations;
};

#endif // LBACKGROUNDBLUR_H
