#ifndef RFRACTIONALSCALE_H
#define RFRACTIONALSCALE_H

#include <LResource.h>

class Louvre::Protocols::FractionalScale::RFractionalScale final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept
    {
        return m_surfaceRes.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void preferredScale(Float32 scale) noexcept;

private:
    friend class Louvre::Protocols::FractionalScale::GFractionalScaleManager;
    RFractionalScale(Wayland::RSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RFractionalScale() noexcept = default;
    LWeak<Wayland::RSurface> m_surfaceRes;
    Float32 m_lastScale { -1.f };
};

#endif // RFRACTIONALSCALE_H
