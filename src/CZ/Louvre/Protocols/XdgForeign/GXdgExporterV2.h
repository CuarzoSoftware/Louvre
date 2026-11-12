#ifndef GXDGEXPORTERV2_H
#define GXDGEXPORTERV2_H

#include <CZ/Louvre/LResource.h>

namespace CZ::Protocols::XdgForeign
{
    class RXdgExportedV2;
}

class CZ::Protocols::XdgForeign::GXdgExporterV2 final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void export_toplevel(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept;

private:
    LGLOBAL_INTERFACE
    GXdgExporterV2(wl_client *client, Int32 version, UInt32 id);
    ~GXdgExporterV2() noexcept;
};

#endif // GXDGEXPORTERV2_H
