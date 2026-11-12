#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgExporterV1.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LClient.h>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_exported_v1_interface imp
{
    .destroy = &RXdgExportedV1::destroy
};

RXdgExportedV1::RXdgExportedV1(GXdgExporterV1 *exporterRes, UInt32 id, LToplevelRole *toplevel) noexcept
    :RXdgExportedBase
    (
        exporterRes->client()->client(),
        &zxdg_exported_v1_interface,
        exporterRes->version(),
        id,
        toplevel,
        &imp
    )
{
    zxdg_exported_v1_send_handle(resource(), handle().c_str());
}

/******************** REQUESTS ********************/

void RXdgExportedV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
