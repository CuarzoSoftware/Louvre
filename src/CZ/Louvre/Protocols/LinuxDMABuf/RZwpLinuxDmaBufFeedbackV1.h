#ifndef CZ_RZWPLINUXDMABUFFEEDBACKV1_H
#define CZ_RZWPLINUXDMABUFFEEDBACKV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::LinuxDMABuf::RZwpLinuxDmaBufFeedbackV1 final : public LResource
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
    friend class CZ::Protocols::LinuxDMABuf::GZwpLinuxDmaBufV1;
    RZwpLinuxDmaBufFeedbackV1(GZwpLinuxDmaBufV1 *linuxDMABufRes, UInt32 id) noexcept;
    ~RZwpLinuxDmaBufFeedbackV1() noexcept = default;
};

#endif // CZ_RZWPLINUXDMABUFFEEDBACKV1_H
