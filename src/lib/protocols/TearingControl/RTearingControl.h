#ifndef RTEARINGCONTROL_H
#define RTEARINGCONTROL_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::TearingControl::RTearingControl final : public LResource
{
public:

    Wayland::RSurface *surfaceRes() const noexcept
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
    friend class Louvre::Protocols::TearingControl::GTearingControlManager;
    RTearingControl(Wayland::RSurface *surfaceRes, Int32 version, UInt32 id) noexcept;
    ~RTearingControl() noexcept = default;
    LWeak<Wayland::RSurface> m_surfaceRes;
    bool m_preferVSync { true };
};

#endif // RTEARINGCONTROL_H
