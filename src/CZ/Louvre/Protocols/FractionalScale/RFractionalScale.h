#ifndef RFRACTIONALSCALE_H
#define RFRACTIONALSCALE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::FractionalScale::RFractionalScale final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void preferredScale(Float32 scale) noexcept;

private:
    friend class GFractionalScaleManager;
    RFractionalScale(Wayland::RSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RFractionalScale() noexcept = default;
    CZWeak<Wayland::RSurface> m_surfaceRes;
    Float32 m_lastScale { -1.f };
};

#endif // RFRACTIONALSCALE_H
