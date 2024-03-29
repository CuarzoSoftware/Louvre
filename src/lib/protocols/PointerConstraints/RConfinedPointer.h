#ifndef RCONFINEDPOINTER_H
#define RCONFINEDPOINTER_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PointerConstraints::RConfinedPointer final : public LResource
{
public:

    UInt32 lifetime() const noexcept
    {
        return m_lifetime;
    }

    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

    bool constrained() const noexcept
    {
        return m_constrained;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_region(wl_client *client, wl_resource *resource, wl_resource *region) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void confined() noexcept;
    void unconfined();

private:
    friend class Louvre::Protocols::PointerConstraints::GPointerConstraints;
    RConfinedPointer(GPointerConstraints *pointerConstraintsRes,
                   LSurface *surface,
                   Wayland::RPointer *pointerRes,
                   Wayland::RRegion *regionRes,
                   UInt32 lifetime,
                   UInt32 id) noexcept;
    ~RConfinedPointer();
    LWeak<Wayland::RPointer> m_pointerRes;
    LWeak<LSurface> m_surface;
    UInt32 m_lifetime;
    bool m_constrained { false };
};

#endif // RCONFINEDPOINTER_H
