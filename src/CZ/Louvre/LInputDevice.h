#ifndef LINPUTDEVICE_H
#define LINPUTDEVICE_H

#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LObject.h>

/**
 * @brief Input Device
 *
 * @anchor linputdevice_detailed
 *
 * This class provides basic information about an input device and a way to access its underlying backend handle
 * so that it can be configured using backend-specific APIs.
 *
 * Use LSeat::inputDevices() to access all available devices, and LSeat::inputDevicePlugged() and LSeat::inputDeviceUnplugged()
 * to listen for hotplug events.
 *
 * Each subtype of LInputEvent provides access to the input device that generated it through LInputEvent::device().
 *
 * @note LInputDevice instances are not destroyed when the device is unplugged, but its nativeHandle() will return `nullptr`.
 *
 * ### Capabilities
 *
 * Devices are not categorized by type but by their capabilities. Use hasCapability() to check for a specific capability.
 *
 * ### Configuration
 *
 * Louvre does not provide a generic API for configuring input devices. Instead, nativeHandle() can be used to access
 * the data structure used by the current input backend. For example, if the @ref LInputBackendLibinput backend is used, the native handle represents
 * a [libinput_device](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput__device.html) struct, which can be configured
 * through the libinput API.
 *
 * Use LCompositor::inputBackendId() to check which input backend is currently loaded.
 *
 * @warning nativeHandle() can return `nullptr` if the input backend does not provide a handle or if the input device has been unplugged.
 */
class Louvre::LInputDevice
{
public:

    /**
     * @brief Input device capabilities
     */
    enum Capability : UInt32
    {
        /// Pointer
        Pointer = 0,

        /// Keyboard
        Keyboard = 1,

        /// Touch
        Touch = 2,

        /// Tablet Tool
        TabletTool = 3,

        /// Tablet Pad
        TabletPad = 4,

        /// Gestures
        Gestures = 5,

        /// Switch
        Switch = 6,
    };

    LCLASS_NO_COPY(LInputDevice)

    /**
     * @brief Checks if the device has the given capability.
     *
     * @param capability The capability to check.
     * @return `true` if the device has the capability, otherwise `false`.
     */
    bool hasCapability(Capability capability) const noexcept
    {
        return m_capabilities & (1 << capability);
    }

    /**
     * @brief Gets the name of the input device.
     */
    const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * @brief Gets the product ID of the input device.
     */
    UInt32 productId() const noexcept
    {
        return m_productId;
    }

    /**
     * @brief Gets the vendor ID of the input device.
     */
    UInt32 vendorId() const noexcept
    {
        return m_vendorId;
    }

    /**
     * @brief Native data structure used by the current input backend.
     *
     * - If the backend is @ref LInputBackendLibinput, it returns a pointer to a `libinput_device` struct.
     * - If the backend is @ref LInputBackendWayland, it returns `nullptr`.
     *
     * @see LCompositor::inputBackendContextHandle().
     *
     * @warning This method can return `nullptr` in cases where the input backend does not provide a handle or the input device has been unplugged.
     *
     * @return A pointer to the native data structure.
     */
    void *nativeHandle() const noexcept
    {
        return m_nativeHandle;
    }

private:
    LInputDevice(UInt32 capabilities = 0,
                 const std::string &name = "Unknown",
                 UInt32 vendorId = 0,
                 UInt32 productId = 0,
                 void *nativeHandle = nullptr) noexcept :
        m_capabilities(capabilities),
        m_name(name),
        m_vendorId(vendorId),
        m_productId(productId),
        m_nativeHandle(nativeHandle)
    {}
    friend class LCompositor;
    friend class LInputBackend;
    UInt32 m_capabilities;
    std::string m_name;
    UInt32 m_vendorId;
    UInt32 m_productId;
    void *m_nativeHandle;
    void notifyPlugged();
    void notifyUnplugged();
};

#endif // LINPUTDEVICE_H
