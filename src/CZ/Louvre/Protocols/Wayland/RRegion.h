#ifndef RREGION_H
#define RREGION_H

#include <CZ/Louvre/LResource.h>
#include <CZ/skia/core/SkRegion.h>

class Louvre::Protocols::Wayland::RRegion final : public LResource
{
public:
    const SkRegion &region() const noexcept
    {
        if (!m_subtract.isEmpty())
        {
            m_region.op(m_subtract, SkRegion::kDifference_Op);
            m_subtract.setEmpty();
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
    mutable SkRegion m_region;
    mutable SkRegion m_subtract;
};

#endif // RREGION_H
