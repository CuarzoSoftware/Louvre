#ifndef GSUBCOMPOSITOR_H
#define GSUBCOMPOSITOR_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GSubcompositor final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_subsurface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent) noexcept;

private:
    GSubcompositor(wl_client *client,
                   Int32 version,
                   UInt32 id) noexcept;

    ~GSubcompositor() noexcept;
};

#endif // GSUBCOMPOSITOR_H
