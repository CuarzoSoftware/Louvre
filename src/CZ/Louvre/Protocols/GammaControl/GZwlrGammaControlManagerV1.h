#ifndef GZWLRGAMMACONTROLMANAGERV1_H
#define GZWLRGAMMACONTROLMANAGERV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::GammaControl::GZwlrGammaControlManagerV1 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_gamma_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output) noexcept;
private:
    LGLOBAL_INTERFACE
    GZwlrGammaControlManagerV1(wl_client *client, Int32 version, UInt32 id);
    ~GZwlrGammaControlManagerV1() noexcept;
};

#endif // GZWLRGAMMACONTROLMANAGERV1_H
