#ifndef RTEARINGCONTROL_H
#define RTEARINGCONTROL_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::TearingControl::RTearingControl final : public LResource
{
public:

    Wayland::RWlSurface *surfaceRes() const noexcept
    {
        return m_surfaceRes;
    }

    bool preferVSync() const noexcept
    {
        return m_preferVSync;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_presentation_hint(wl_client *client, wl_resource *resource, UInt32 hint) noexcept;

private:
    friend class CZ::Protocols::TearingControl::GTearingControlManager;
    RTearingControl(Wayland::RWlSurface *surfaceRes, Int32 version, UInt32 id) noexcept;
    ~RTearingControl() noexcept = default;
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
    bool m_preferVSync { true };
};

#endif // RTEARINGCONTROL_H
