#ifndef RREGION_H
#define RREGION_H

#include <LResource.h>
#include <LRegion.h>

class Louvre::Protocols::Wayland::RRegion final : public LResource
{
public:
    const LRegion &region() const noexcept
    {
        if (!m_subtract.empty())
        {
            pixman_region32_subtract(&m_region.m_region, &m_region.m_region, &m_subtract.m_region);
            m_subtract.clear();
        }

        return m_region;
    };

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept;
    static void subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GCompositor;
    RRegion(GCompositor *compositorRes, UInt32 id) noexcept;
    ~RRegion() noexcept = default;
    mutable LRegion m_region;
    mutable LRegion m_subtract;
};

#endif // RREGION_H
