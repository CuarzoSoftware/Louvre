#ifndef GGAMMACONTROLMANAGER_H
#define GGAMMACONTROLMANAGER_H

#include <LResource.h>
#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>

class Louvre::Protocols::GammaControl::GGammaControlManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_gamma_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output) noexcept;

private:
    GGammaControlManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GGammaControlManager() noexcept;
};

#endif // GGAMMACONTROLMANAGER_H
