#ifndef GLINUXDMABUF_H
#define GLINUXDMABUF_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::LinuxDMABuf::GLinuxDMABuf final : public LResource
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
    GLinuxDMABuf(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GLinuxDMABuf() noexcept;
};

#endif // GLINUXDMABUF_H
