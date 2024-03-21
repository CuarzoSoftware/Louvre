#ifndef GFRACTIONALSCALEMANAGER_H
#define GFRACTIONALSCALEMANAGER_H

#include <LResource.h>
#include <protocols/FractionalScale/fractional-scale-v1.h>

class Louvre::Protocols::FractionalScale::GFractionalScaleManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_fractional_scale(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;

private:
    GFractionalScaleManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GFractionalScaleManager() noexcept;
};

#endif // GFRACTIONALSCALEMANAGER_H
