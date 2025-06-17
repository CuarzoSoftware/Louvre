#ifndef RLINUXDMABUFFEEDBACK_H
#define RLINUXDMABUFFEEDBACK_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::LinuxDMABuf::RLinuxDMABufFeedback final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void done() noexcept;
    void formatTable(Int32 fd, UInt32 size) noexcept;
    void mainDevice(wl_array *devices) noexcept;
    void trancheDone() noexcept;
    void trancheTargetDevice(wl_array *devices) noexcept;
    void trancheFormats(wl_array *indices) noexcept;
    void trancheFlags(UInt32 flags) noexcept;

private:
    friend class Louvre::Protocols::LinuxDMABuf::GLinuxDMABuf;
    RLinuxDMABufFeedback(GLinuxDMABuf *linuxDMABufRes, UInt32 id) noexcept;
    ~RLinuxDMABufFeedback() noexcept = default;
};

#endif // RLINUXDMABUFFEEDBACK_H
