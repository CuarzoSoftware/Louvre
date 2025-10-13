#ifndef CZ_WPGSINGLEPIXELBUFFERV1_H
#define CZ_WPGSINGLEPIXELBUFFERV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::SinglePixelBuffer::GWpSinglePixelBufferManagerV1 final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_u32_rgba_buffer(wl_client *client, wl_resource *resource, UInt32 id, UInt32 r, UInt32 g, UInt32 b, UInt32 a) noexcept;

private:
    LGLOBAL_INTERFACE
    GWpSinglePixelBufferManagerV1(wl_client *client, Int32 version, UInt32 id);
    ~GWpSinglePixelBufferManagerV1() noexcept;
};

#endif // CZ_WPGSINGLEPIXELBUFFERV1_H
