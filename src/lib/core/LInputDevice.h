#ifndef LINPUTDEVICE_H
#define LINPUTDEVICE_H

#include <LSeat.h>
#include <LObject.h>

/**
 * @brief Input Device
 *
 * @anchor linputdevice_detailed
 *
 * This class represents an input device, providing essential information for identification.\n
 * The complete list of available devices can be accessed through LSeat::inputDevices().\n
 * To listen to hot-plugging events, use LSeat::inputDevicePlugged() and LSeat::inputDeviceUnplugged().\n
 * Each subtype of LInputEvent provides access to the input device that generated it through LInputEvent::device().
 *
 * @note LInputDevice instances are not destroyed when the device is unplugged, but its nativeHandle() returns `nullptr`.
 *
 * ### Capabilities
 *
 * Devices are not categorized by type, but by their capabilities. Each device may have more than one, use hasCapability() to check for a specific capability.
 *
 * ### Configuration
 *
 * Louvre does not provide a generic API for configuring input device parameters. Instead, nativeHandle() can be employed to access
 * the data structure used by the input backend. For example, if the LInputBackendLibinput backend is used, the native handle represents
 * a [libinput_device](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput__device.html) struct, which can be configured
 * through the libinput API.
 *
 * @warning nativeHandle() can return `nullptr` in cases where the input backend does not provide a handle or the input device has been unplugged.
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
     * @brief Checks if the device has a given capability.
     *
     * @param capability The capability to check.
     * @return `true` if the device has the capability, otherwise `false`.
     */
    bool hasCapability(Capability capability) const noexcept
    {
        return m_capabilities & (1 << capability);
    }

    /**
     * @brief Get the name of the input device.
     */
    const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * @brief Get the product ID of the input device.
     */
    UInt32 productId() const noexcept
    {
        return m_productId;
    }

    /**
     * @brief Get the vendor ID of the input device.
     */
    UInt32 vendorId() const noexcept
    {
        return m_vendorId;
    }

    /**
     * @brief Provides access to the native data structure used by the input backend.
     *
     * If the backend is LInputBackendLibinput, it returns a pointer to a `libinput_device` struct.\n
     * If the backend is LInputBackendWayland, it returns `nullptr`.
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
