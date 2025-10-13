#ifndef LBACKGROUNDBLUR_H
#define LBACKGROUNDBLUR_H

#include <CZ/skia/core/SkRegion.h>
#include <CZ/skia/core/SkPath.h>
#include <CZ/Louvre/LFactoryObject.h>
#include <CZ/Core/CZRRect.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZColorScheme.h>
#include <list>

/**
 * @brief Background blur controller for surfaces.
 *
 * This class allows configuring the background blur effect for a surface and listening to client changes. See LSurface::backgroundBlur().
 *
 * When a client wants to apply a blur effect to a surface:
 * 1. The `configureRequest()` is triggered, typically before the surface is mapped.
 * 2. The compositor configures the blur state.
 * 3. The client specifies the region to blur and may optionally apply an additional clipping mask.
 * 4. The client acknowledges and commits the changes, which are notified via `propsChanged()`.
 *
 * The final blur region is determined by the intersection of the surface bounds, the region(), and the optional clipping mask.
 *
 * The compositor can modify the state and color hint configuration at any time but should always wait for the client’s acknowledgment
 * before applying the changes.
 *
 * Compositors may support different masking capabilities depending on their rendering features. Modify @ref maskingCapabilities to
 * advertise the supported capabilities to clients.
 *
 * @note The SVG path and background blur protocols are experimental and thus not enabled by default in `LCompositor::createGlobalsRequest()`.
 */
class CZ::LBackgroundBlur : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LBackgroundBlur;

    /**
     * @brief Flags indicating the masks supported by the compositor.
     *
     * These flags are used in @ref maskingCapabilities .
     */
    enum MaskingCapabilities : UInt32
    {
        /**
         * @brief No masking capability is supported.
         */
        NoMaskCap        = 0U,

        /**
         * @brief Supports rounded rectangle masking.
         */
        RoundRectMaskCap = 1U,

        /**
         * @brief Supports masking using an SVG path.
         */
        SVGPathMaskCap   = 2U
    };

    /**
     * @brief Current masking capabilities of the compositor.
     *
     * Defines the masking features currently supported by the compositor.
     * This value should ideally be set during compositor initialization.
     *
     * Modifying this value does not affect the masking capabilities
     * announced to clients that have previously created background blur effects.
     *
     * @note This setting applies globally.
     */
    static inline CZBitset<MaskingCapabilities> MaskCaps { NoMaskCap };

    /**
     * @brief Enumeration of possible blur states.
     */
    enum State
    {
        Disabled, ///< Background blur is disabled (default).
        Enabled   ///< Background blur is enabled.
    };

    /**
     * @brief Mask types.
     */
    enum MaskType
    {
        NoMask,   ///< No mask is applied (default).
        RoundRect,///< A rounded rectangle mask is applied.
        SVGPath   ///< An SVG path is used as the mask.
    };

    /**
     * @brief Flags representing the property changes in propsChanged().
     */
    enum PropChanges : UInt8
    {
        /** Indicates that the state has changed. */
        StateChanged          = static_cast<UInt8>(1) << 0,

        /** Indicates that the color hint has changed. */
        ColorSchemeChanged    = static_cast<UInt8>(1) << 1,

        /** Indicates that the region has changed. */
        RegionChanged         = static_cast<UInt8>(1) << 2,

        /** Indicates that the clipping mask has changed. */
        MaskChanged           = static_cast<UInt8>(1) << 3,

        /** Indicates that the serial has changed. */
        SerialChanged         = static_cast<UInt8>(1) << 4,
    };

    /**
     * @brief Represents a configuration sent to the client.
     */
    struct Configuration
    {
        State state { Disabled };        ///< The desired blur state
        UInt32 serial { 0 };             ///< The serial number associated with the configuration
    };

    /**
     * @brief Properties.
     */
    struct Props
    {
        /**
         * @brief The current blur state.
         */
        State state { Disabled };

        /**
         * @brief The current blur color hint.
         */
        CZColorScheme colorScheme { CZColorScheme::Unknown };

        /**
         * @brief The current mask type.
         */
        MaskType maskType { NoMask };

        /**
         * @brief Region in local surface coordinates to be blurred.
         *
         * It is guaranteed that the region does not extend beyond the surface bounds.
         */
        SkRegion region { SkRegion() };

        /**
         * @brief Returns the current rounded rectangle mask, if the mask type is set to @ref RoundRect .
         *
         * The rounded rectangle is guaranteed to be valid, but it may extend beyond the surface bounds.
         *
         * If the compositor does not support rounded rectangle masks, it should remove the @ref RoundRectMaskCap
         * from @ref maskingCapabilities to prevent clients from using it.
         */
        CZRRect roundRectMask;

        /**
         * @brief Returns the current SVG path mask, if the mask type is set to @ref SVGPath .
         *
         * Note that the path may extend beyond the surface bounds.
         *
         * @warning Louvre does not validate the SVG commands within the string. It is the
         * responsibility of the caller to ensure the SVG path is valid.
         *
         * If the compositor does not support SVG path masks, it should remove the @ref SVGPathMaskCap
         * from @ref maskingCapabilities to prevent clients from using it.
         */
        SkPath svgPathMask;

        /**
         * @brief Returns the last acknowledged configuration serial.
         */
        UInt32 serial { 0 };

        /**
         * @brief Indicates whether the blur region is empty.
         *
         * Equivalent to `region().empty()`.
         */
        bool isEmpty { true };

        /**
         * @brief Indicates whether the blur region covers the entire surface.
         *
         * Returns `true` if region() contains a single rectangle matching the current surface bounds.
         */
        bool isFullSize { false };
    };

    /**
     * @brief Constructor.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LBackgroundBlur(const void *params) noexcept;

    /**
     * @brief Destructor.
     */
    ~LBackgroundBlur() noexcept = default;

    /**
     * @brief Handle to the `lvr_background_blur` resource wrapper.
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
     * The property values can also be accessed directly via aliases such as state(), colorHint(), maskType(), etc.
     */
    const Props &props() const noexcept { return m_props[m_currentPropsIndex]; };

    /**
     * @brief Indicates whether the blur effect should be displayed.
     *
     * The effect should be applied when the client has acknowledged an @ref Enabled state
     * and the blur region() is not empty.
     */
    bool visible() const noexcept { return props().state && !props().isEmpty; };

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
        if (!supported() || state == m_pendingConfiguration.state)
            return;
        updateSerial();
        m_flags.add(HasStateToSend);
        m_pendingConfiguration.state = state;
    }

    /**
     * @brief Triggered when the client creates a background blur resource for the surface.
     *
     * The compositor should respond by configuring both the blur state and color hint.
     * If no configuration is explicitly sent, Louvre will automatically apply
     * the current pending configuration values, which default to @ref Disabled for the state
     * and @ref Unknown for the color hint.
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
     * @param prevProps The state of the properties before the change.
     *
     * #### Default implementation
     * @snippet LBackgroundBlurDefault.cpp propsChanged
     */
    virtual void propsChanged(CZBitset<PropChanges> changes, const Props &prevProps);

    /**
     * @brief Retrieves the associated surface.
     */
    LSurface *surface() const noexcept { return &m_surface; };
private:
    friend class LCompositor;
    friend class LSurface;
    friend class Protocols::BackgroundBlur::RBackgroundBlur;
    friend class Protocols::Wayland::RWlSurface;

    enum Flags : UInt8
    {
        HasStateToSend       = static_cast<UInt8>(1) << 0,
        SchemeModified       = static_cast<UInt8>(1) << 1,
        RegionModified       = static_cast<UInt8>(1) << 2,
        MaskModified         = static_cast<UInt8>(1) << 3,
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
    CZWeak<Protocols::BackgroundBlur::RBackgroundBlur> m_backgroundBlurRes;
    CZBitset<Flags> m_flags;
    Props m_props[2];
    UInt8 m_currentPropsIndex { 0 };
    Configuration m_pendingConfiguration;
    std::list<Configuration> m_sentConfigurations;
};

#endif // LBACKGROUNDBLUR_H
