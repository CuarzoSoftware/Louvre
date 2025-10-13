#ifndef GWLDRM_H
#define GWLDRM_H

#include <CZ/Ream/Ream.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::WaylandDRM::GWlDRM final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void authenticate(wl_client *client, wl_resource *resource, UInt32 magic);
    static void create_buffer(wl_client *client, wl_resource *resource, UInt32 id, UInt32 name, Int32 width,
                              Int32 height, UInt32 stride, UInt32 format);
    static void create_planar_buffer(wl_client *client, wl_resource *resource, UInt32 id, UInt32 name, Int32 width,
                                     Int32 height, UInt32 format, Int32 offset0, Int32 stride0,
                                     Int32 offset1, Int32 stride1, Int32 offset2, Int32 stride2);
    static void create_prime_buffer(wl_client *client, wl_resource *resource, UInt32 id, int fd, Int32 width,
                                    Int32 height, UInt32 format, Int32 offset0, Int32 stride0,
                                    Int32 offset1, Int32 stride1, Int32 offset2, Int32 stride2);

    /******************** EVENTS ********************/

    void authenticated() noexcept;
    void device(const char *name) noexcept;
    void format(RFormat format) noexcept;
    void capabilities(UInt32 caps) noexcept;
private:
    LGLOBAL_INTERFACE
    bool sentRenderNode;
    GWlDRM(wl_client *client, Int32 version, UInt32 id);
    ~GWlDRM() noexcept;
};

#endif // GWLDRM_H
