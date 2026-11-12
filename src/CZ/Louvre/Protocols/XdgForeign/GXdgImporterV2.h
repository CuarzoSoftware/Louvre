#ifndef GXDGIMPORTERV2_H
#define GXDGIMPORTERV2_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::XdgForeign::GXdgImporterV2 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void import_toplevel(wl_client *client, wl_resource *resource, UInt32 id, const char *handle) noexcept;

private:
    LGLOBAL_INTERFACE
    GXdgImporterV2(wl_client *client, Int32 version, UInt32 id);
    ~GXdgImporterV2() noexcept;
};

#endif // GXDGIMPORTERV2_H
