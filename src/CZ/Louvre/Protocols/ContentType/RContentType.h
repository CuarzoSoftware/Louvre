#ifndef RCONTENTTYPE_H
#define RCONTENTTYPE_H

#include <LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::ContentType::RContentType final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept { return m_surfaceRes; }


    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_content_type(wl_client *client, wl_resource *resource, UInt32 type);

private:
    friend class GContentTypeManager;
    RContentType(Wayland::RSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RContentType() noexcept;
    CZWeak<Wayland::RSurface> m_surfaceRes;
};

#endif // RCONTENTTYPE_H
