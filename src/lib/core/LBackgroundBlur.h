#ifndef LBACKGROUNDBLUR_H
#define LBACKGROUNDBLUR_H

#include <LRegion.h>
#include <LRRect.h>
#include <LFactoryObject.h>
#include <LBitset.h>
#include <LWeak.h>
#include <list>
#include <string>

/**
 * @brief Background blur controller for surfaces.
 *
 * This class allows configuring the background blur effect for a surface and listening to client changes. See LSurface::backgroundBlur().
 *
 * When a client wants to apply a blur effect to a surface:
 * 1. The `configureRequest()` is triggered, typically before the surface is mapped.
 * 2. The compositor configures the blur state and style.
 * 3. The client specifies the region to blur and may optionally apply an additional clipping mask.
 * 4. The client acknowledges and commits the changes, which are notified via `propsChanged()`.
 *
 * The final blur region is determined by the intersection of the surface bounds, the region(), and the clipping mask.
 *
 * The compositor can modify the configuration at any time but should always wait for the client’s acknowledgment
 * before applying the changes.
 *
 * @note The background blur protocol is experimental and thus not enabled by default in `LCompositor::createGlobalsRequest()`.
 */
class Louvre::LBackgroundBlur : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LBackgroundBlur;

    /**
     * @brief Enumeration of possible blur states.
     */
    enum State
    {
        Disabled, ///< Background blur is disabled (default).
        Enabled   ///< Background blur is enabled.
    };

    /**
     * @brief The background blur color tone.
     */
    enum Style
    {
        Dark,    ///< Dark.
        Light    ///< Light (default).
    };

    /**
     * @brief Clip mask types.
     */
    enum ClipType
    {
        NoClip,   ///< No clip mask is applied (default).
        RoundRect,///< A rounded rectangle clip mask is applied.
        SVGPath   ///< An SVG path is used as the clip mask.
    };

    /**
     * @brief Flags representing the property changes in propsChanged().
     */
    enum PropChanges : UInt8
    {
        /** Indicates that the state has changed. */
        StateChanged          = static_cast<UInt8>(1) << 0,

        /** Indicates that the style has changed. */
        StyleChanged          = static_cast<UInt8>(1) << 1,

        /** Indicates that the region has changed. */
        RegionChanged         = static_cast<UInt8>(1) << 2,

        /** Indicates that the clip has changed. */
        ClipChanged           = static_cast<UInt8>(1) << 3,

        /** Indicates that the serial has changed. */
        SerialChanged         = static_cast<UInt8>(1) << 4,
    };

    /**
     * @brief Represents a configuration sent to the client.
     */
    struct Configuration
    {
        State state { Disabled };    ///< The desired blur state
        Style style { Light };       ///< The desired blur style
        UInt32 serial { 0 };         ///< The serial number associated with the configuration
    };

    /**
     * @brief Properties.
     */
    struct Props
    {
        State state { Disabled };                ///< The current blur state
        Style style { Light };                   ///< The current blur style
        ClipType clipType { NoClip };            ///< The clip type
        LRegion region { LRegion() };            ///< The region to be blurred
        LRRect roundRectClip;                    ///< The rounded rectangle clip (if applicable)
        std::string svgPathClip;                 ///< The SVG path clip (if applicable)
        UInt32 serial { 0 };                     ///< The serial number associated with the configuration
        bool isEmpty { true };                   ///< Whether the region is empty
        bool isFullSize { false };               ///< Whether the region covers the full surface
    };

    /**
     * @brief Constructor.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LBackgroundBlur(const void *params) noexcept;

    LCLASS_NO_COPY(LBackgroundBlur)

    /**
     * @brief Destructor.
     */
    ~LBackgroundBlur() noexcept = default;

    /**
     * @brief Handle to the `background_blur` Wayland resource wrapper.
     *
     * @returns `nullptr` if the protocol is not supported()
     */
    Protocols::BackgroundBlur::RBackgroundBlur *backgroundBlurResource() const noexcept;

    /**
     * @brief Indicates whether the surface supports the protocol.
     *
     * @note If the client stops using the protocol, props() reverts to the default values, triggering propsChanged() when applicable.
     */
    bool supported() const noexcept { return backgroundBlurResource() != nullptr; };

    /**
     * @brief Returns the last sent configuration parameters.
     *
     * If the serial is equal to the current serial(), it means there is no pending configuration.
     */
    const Configuration &pendingConfiguration() const noexcept { return m_pendingConfiguration; };

    /**
     * @brief The current properties (acknowledged by the client).
     *
     * The property values can also be accessed directly via aliases such as state(), style(), clipType(), etc.
     */
    const Props &props() const noexcept { return m_props[m_currentPropsIndex]; };

    /**
     * @brief The current blur state.
     */
    State state() const noexcept { return props().state; };

    /**
     * @brief The current blur style.
     */
    Style style() const noexcept { return props().style; };

    /**
     * @brief The current clip mode.
     */
    ClipType clipType() const noexcept { return props().clipType; };

    /**
     * @brief Region in local surface coordinates to be blurred.
     *
     * It is guaranteed that the region does not extend beyond the surface bounds.
     */
    const LRegion &region() const noexcept { return props().region; };

    /**
     * @brief Returns the current clip as a rounded rectangle, if the clip type is set to @ref RoundRect.
     *
     * The rounded rectangle is guaranteed to be valid, but it may extend beyond the surface bounds.
     *
     * If the compositor does not support rounded rectangle clips, it should limit the
     * `GBackgroundBlurManager` global to version 1.
     */
    const LRRect &roundRectClip() const noexcept { return props().roundRectClip; };

    /**
     * @brief Returns the current clip SVG path, if the clip type is set to @ref SVGPath.
     *
     * Note that the path may extend beyond the surface bounds.
     *
     * @warning Louvre does not validate the SVG commands within the string. It is the
     * responsibility of the caller to ensure the SVG path is valid.
     *
     * If the compositor does not support SVG paths, it should set the
     * `GBackgroundBlurManager` global to a version below 3.
     */
    const std::string &svgPathClip() const noexcept { return props().svgPathClip; };

    /**
     * @brief Returns the last acknowledged serial.
     */
    UInt32 serial() const noexcept { return props().serial; };

    /**
     * @brief Indicates whether the blur region is empty.
     *
     * Equivalent to `region().empty()`.
     */
    bool isEmpty() const noexcept { return props().isEmpty; };

    /**
     * @brief Indicates whether the blur region covers the entire surface.
     *
     * Returns `true` if region() contains a single rectangle matching the current surface bounds.
     */
    bool isFullSize() const noexcept { return props().isFullSize; };

    /**
     * @brief Indicates whether the blur effect should be displayed.
     *
     * The effect is visible when the client has acknowledged an @ref Enabled state
     * and the blur region() is not empty.
     */
    bool visible() const noexcept { return state() && !isEmpty(); };

    /**
     * @brief Notifies the client of the blur state.
     *
     * This is an asynchronous operation. The current state should be maintained
     * until the client acknowledges the change via propsChanged().
     *
     * The configuration is immediately stored in pendingConfiguration().
     *
     * If blur is not supported() calling this function is a no-op.
     */
    void configureState(State state) noexcept
    {
        if (!supported())
            return;
        updateSerial();
        m_flags.add(HasStateToSend);
        m_pendingConfiguration.state = state;
    }

    /**
     * @brief Notifies the client of the background color tone.
     *
     * Clients can use this hint, for example, to set the overlay text color to white when the style is @ref Dark .
     *
     * This is an asynchronous operation. The current style should be maintained
     * until the client acknowledges the change via propsChanged().
     *
     * The configuration is immediately stored in pendingConfiguration().
     *
     * If blur is not supported() calling this function is a no-op.
     */
    void configureStyle(Style style) noexcept
    {
        if (!supported())
            return;
        updateSerial();
        m_flags.add(HasStyleToSend);
        m_pendingConfiguration.style = style;
    }

    /**
     * @brief Triggered when the client creates a background blur resource for the surface.
     *
     * The compositor should respond by configuring both the blur state and style.
     * If no configuration is explicitly sent, Louvre will automatically apply
     * the current pending configuration values, which default to @ref Disabled for the state
     * and @ref Light for the style.
     *
     * #### Default implementation
     * @snippet LBackgroundBlurDefault.cpp configureRequest
     */
    virtual void configureRequest();

    /**
     * @brief Notifies changes of the current properties.
     *
     * Triggered when the client acknowledges and commits a configuration,
     * or when it requests a change.
     *
     * @param changes   A bitset indicating which properties have changed.
     * @param prevProps The previous state of the properties before the change.
     *
     * #### Default implementation
     * @snippet LBackgroundBlurDefault.cpp propsChanged
     */
    virtual void propsChanged(LBitset<PropChanges> changes, const Props &prevProps);

    /**
     * @brief Retrieves the associated surface.
     */
    LSurface *surface() const noexcept { return &m_surface; };
private:
    friend class LCompositor;
    friend class Protocols::BackgroundBlur::RBackgroundBlur;
    friend class Protocols::Wayland::RSurface;

    enum Flags : UInt8
    {
        HasStateToSend       = static_cast<UInt8>(1) << 0,
        HasStyleToSend       = static_cast<UInt8>(1) << 1,
        RegionModified       = static_cast<UInt8>(1) << 2,
        ClipModified         = static_cast<UInt8>(1) << 3,
        Destroyed            = static_cast<UInt8>(1) << 4,
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
