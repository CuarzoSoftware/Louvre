#ifndef RGAMMACONTROL_H
#define RGAMMACONTROL_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::GammaControl::RGammaControl final : public LResource
{
public:

    Wayland::GOutput *outputRes() const noexcept
    {
        return m_outputRes;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_gamma(wl_client *client, wl_resource *resource, Int32 fd);

    /******************** EVENTS ********************/

    // Since 1
    void gammaSize(UInt32 size) noexcept;
    void failed() noexcept;

private:
    friend class Louvre::Protocols::GammaControl::GGammaControlManager;
    RGammaControl(Wayland::GOutput *outputRes, Int32 version, UInt32 id) noexcept;
    ~RGammaControl();
    LWeak<Wayland::GOutput> m_outputRes;
};

#endif // RGAMMACONTROL_H
