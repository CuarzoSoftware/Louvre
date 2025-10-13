#ifndef CZ_GWPCONTENTTYPEMANAGERV1_H
#define CZ_GWPCONTENTTYPEMANAGERV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::ContentType::GWpContentTypeManagerV1 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_surface_content_type(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

private:
    LGLOBAL_INTERFACE
    GWpContentTypeManagerV1(wl_client *client, Int32 version, UInt32 id);
    ~GWpContentTypeManagerV1() noexcept;
};

#endif // CZ_GWPCONTENTTYPEMANAGERV1_H
