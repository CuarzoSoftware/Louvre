#ifndef CZ_GZWPLINUXDMABUFV1_H
#define CZ_GZWPLINUXDMABUFV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::LinuxDMABuf::GZwpLinuxDmaBufV1 final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_params(wl_client *client, wl_resource *resource, UInt32 id) noexcept;

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
    static void get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
#endif

    /******************** EVENTS ********************/

    // Since 1
    void format(UInt32 format) noexcept;

    // Since 3
    bool modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo) noexcept;

private:
    LGLOBAL_INTERFACE
    GZwpLinuxDmaBufV1(wl_client *client, Int32 version, UInt32 id);
    ~GZwpLinuxDmaBufV1() noexcept;
};

#endif // CZ_GZWPLINUXDMABUFV1_H
