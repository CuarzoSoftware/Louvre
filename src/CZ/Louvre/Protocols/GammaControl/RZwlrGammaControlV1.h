#ifndef RZWLRGAMMACONTROLV1_H
#define RZWLRGAMMACONTROLV1_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::GammaControl::RZwlrGammaControlV1 final : public LResource
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
    friend class CZ::Protocols::GammaControl::GZwlrGammaControlManagerV1;
    RZwlrGammaControlV1(Wayland::GOutput *outputRes, Int32 version, UInt32 id) noexcept;
    ~RZwlrGammaControlV1();
    CZWeak<Wayland::GOutput> m_outputRes;
};

#endif // RZWLRGAMMACONTROLV1_H
