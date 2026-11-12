#ifndef GXDGIMPORTERV1_H
#define GXDGIMPORTERV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::XdgForeign::GXdgImporterV1 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void import_toplevel(wl_client *client, wl_resource *resource, UInt32 id, const char *handle) noexcept;

private:
    LGLOBAL_INTERFACE
    GXdgImporterV1(wl_client *client, Int32 version, UInt32 id);
    ~GXdgImporterV1() noexcept;
};

#endif // GXDGIMPORTERV1_H
