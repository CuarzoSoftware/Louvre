#ifndef SUBSURFACE_H
#define SUBSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <memory>

class Louvre::Protocols::Wayland::RSubsurface final : public LResource
{
public:
    LSubsurfaceRole *subsurfaceRole() const noexcept
    {
        return m_subsurfaceRole.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_position(wl_client *client, wl_resource *resource, Int32 x, Int32 y);
    static void place_above(wl_client *client, wl_resource *resource, wl_resource *sibiling);
    static void place_below(wl_client *client, wl_resource *resource, wl_resource *sibiling);
    static void set_sync(wl_client *client, wl_resource *resource);
    static void set_desync(wl_client *client, wl_resource *resource);

private:
    friend class Louvre::Protocols::Wayland::GSubcompositor;
    RSubsurface(GSubcompositor *subcompositorRes,
                LSurface *surface,
                LSurface *parent,
                UInt32 id);
    ~RSubsurface();

    std::unique_ptr<LSubsurfaceRole> m_subsurfaceRole;
};

#endif // SUBSURFACE_H
