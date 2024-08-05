#ifndef RIDLEINHIBITOR_H
#define RIDLEINHIBITOR_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::IdleInhibit::RIdleInhibitor final : public LResource
{
public:

    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

    static void handleRemoval(RIdleInhibitor *idleInhibitorRes, LSurface *surface) noexcept;

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    friend class Louvre::Protocols::IdleInhibit::GIdleInhibitManager;
    RIdleInhibitor(LSurface *surface, Int32 version, UInt32 id) noexcept;
    ~RIdleInhibitor();
    LWeak<LSurface> m_surface;
};

#endif // RIDLEINHIBITOR_H
