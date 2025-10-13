#ifndef CZ_RWPCONTENTTYPEV1_H
#define CZ_RWPCONTENTTYPEV1_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::ContentType::RWpContentTypeV1 final : public LResource
{
public:
    Wayland::RWlSurface *surfaceRes() const noexcept { return m_surfaceRes; }


    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_content_type(wl_client *client, wl_resource *resource, UInt32 type);

private:
    friend class GWpContentTypeManagerV1;
    RWpContentTypeV1(Wayland::RWlSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RWpContentTypeV1() noexcept;
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
};

#endif // CZ_RWPCONTENTTYPEV1_H
