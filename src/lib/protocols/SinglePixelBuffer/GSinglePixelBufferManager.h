#ifndef GSINGLEPIXELBUFFER_H
#define GSINGLEPIXELBUFFER_H

#include <LResource.h>

class Louvre::Protocols::SinglePixelBuffer::GSinglePixelBufferManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_u32_rgba_buffer(wl_client *client, wl_resource *resource, UInt32 id, UInt32 r, UInt32 g, UInt32 b, UInt32 a) noexcept;

private:
    LGLOBAL_INTERFACE
    GSinglePixelBufferManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GSinglePixelBufferManager() noexcept;
};

#endif // GSINGLEPIXELBUFFER_H
