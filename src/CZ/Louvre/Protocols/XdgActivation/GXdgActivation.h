#ifndef GXDGACTIVATION_H
#define GXDGACTIVATION_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::XdgActivation::GXdgActivation final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_activation_token(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void activate(wl_client *client, wl_resource *, const char *token, wl_resource *surface);
private:
    LGLOBAL_INTERFACE
    GXdgActivation(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GXdgActivation() noexcept;
};

#endif // GXDGACTIVATION_H
