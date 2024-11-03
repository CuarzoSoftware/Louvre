#ifndef LGPU_H
#define LGPU_H

#include <LGlobal.h>
#include <LWeak.h>
#include <string>

/**
 * @brief GPU Information
 *
 * This class typically contains information about a DRM node, however, its properties
 * could have different meanings when using virtualized backends.
 *
 * To see all available devices, use LSeat::gpus(). For a particular output, use LOutput::gpu().
 */
class Louvre::LGPU final : LObject
{
public:

    /**
     * @brief Gets the GPU name.
     *
     * When using the DRM backend, this can be, e.g., /dev/dri/card0.
     */
    const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * @brief Gets the Unix device identifier.
     */
    dev_t dev() const noexcept
    {
        return m_dev;
    }

    /**
     * @brief Gets the Libseat device ID.
     *
     * @return A Libseat device ID, or -1 if the device was not opened with Libseat.
     */
    int id() const noexcept
    {
        return m_id;
    }

    /**
     * @brief Gets the read-write file descriptor.
     *
     * @warning Do not attempt to close it or use it unless you know what you're doing.
     *
     * @return A read-write file descriptor, or -1 if the backend doesn't provide it.
     */
    int fd() const noexcept
    {
        return m_fd;
    }

    /**
     * @brief Gets the read-only file descriptor.
     *
     * @return A read-only file descriptor, or -1 if the backend doesn't provide it.
     */
    int roFd() const noexcept
    {
        return m_roFd;
    }

private:
    friend class LCompositor;
    friend class LGraphicBackend;
    dev_t m_dev;
    int m_id { -1 };
    int m_fd { -1 };
    int m_roFd { -1 };
    std::string m_name;
    LWeak<LGlobal> m_leaseGlobal;
    void *m_data;
};

#endif // LGPU_H
