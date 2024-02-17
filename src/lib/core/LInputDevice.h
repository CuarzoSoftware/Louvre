#ifndef LINPUTDEVICE_H
#define LINPUTDEVICE_H

#include <LSeat.h>
#include <LObject.h>
#include <libinput.h>

/**
 * @brief Input Device
 *
 * This class represents an input device, offering methods for querying device information and configuring its properties.
 * The complete list of available devices can be accessed through LSeat::inputDevices() or any of the LInputEvent subtypes to identify the originating device.
 *
 * ### Capabilities
 *
 * Devices aren't categorized by type, instead by the events they generate, such as pointer, keyboard, touch events, or a combination of these.
 * To retrieve device capabilities, use the capabilities() method.
 *
 * ### Configuration
 *
 * Configuration methods for device parameters may not be available for all devices. You can ascertain this by checking their return values,
 * which can be one of three outcomes: Success, Unsupported, or Invalid.
 */
class Louvre::LInputDevice : public LObject
{
public:

    /**
     * @brief Enumerates the status of configuration operations.
     *
     * This enumeration defines the possible outcomes of device configuration operations.
     * The values correspond to those provided by the LIBINPUT_CONFIG_STATUS enumeration.
     */
    enum ConfigurationStatus
    {
        /// The configuration operation was successful.
        Success = LIBINPUT_CONFIG_STATUS_SUCCESS,

        /// The configuration operation is not supported on the device.
        Unsupported = LIBINPUT_CONFIG_STATUS_UNSUPPORTED,

        /// The configuration provided is invalid or not allowed.
        Invalid = LIBINPUT_CONFIG_STATUS_INVALID
    };

    /// @cond OMIT
    inline LInputDevice(LSeat::InputCapabilitiesFlags capabilities = 0,
                        const std::string &name = "Unknown",
                        UInt32 vendorId = 0,
                        UInt32 productId = 0,
                        void *backendData = nullptr):
        m_capabilities(capabilities),
        m_name(name),
        m_vendorId(vendorId),
        m_productId(productId),
        m_backendData(backendData)
    {}
    LInputDevice(const LInputDevice&) = delete;
    LInputDevice& operator= (const LInputDevice&) = delete;
    /// @endcond

    /**
     * @brief Get the capabilities of the input device.
     *
     * This method returns the capabilities of the input device, represented as a combination of flags.
     */
    inline LSeat::InputCapabilitiesFlags capabilities() const
    {
        return m_capabilities;
    }

    /**
     * @brief Get the name of the input device.
     */
    inline const std::string &name() const
    {
        return m_name;
    }

    /**
     * @brief Get the product ID of the input device.
     */
    inline UInt32 productId() const
    {
        return m_productId;
    }

    /**
     * @brief Get the vendor ID of the input device.
     */
    inline UInt32 vendorId() const
    {
        return m_vendorId;
    }

    inline void *backendData() const
    {
        return m_backendData;
    }

private:
    friend class LInputBackend;
    LSeat::InputCapabilitiesFlags m_capabilities;
    std::string m_name;
    UInt32 m_vendorId;
    UInt32 m_productId;
    void *m_backendData;
    void notifyPlugged();
    void notifyUnplugged();
};

#endif // LINPUTDEVICE_H
