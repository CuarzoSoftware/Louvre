#ifndef GXDGEXPORTERV1_H
#define GXDGEXPORTERV1_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::XdgForeign::GXdgExporterV1 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void export_toplevel(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *xdg_surface) noexcept;

private:
    LGLOBAL_INTERFACE
    GXdgExporterV1(wl_client *client, Int32 version, UInt32 id);
    ~GXdgExporterV1() noexcept;
};

#endif // GXDGEXPORTERV1_H
