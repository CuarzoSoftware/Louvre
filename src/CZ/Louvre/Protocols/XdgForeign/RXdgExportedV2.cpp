#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v2.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgExporterV2.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_exported_v2_interface imp
{
    .destroy = &RXdgExportedV2::destroy
};

RXdgExportedV2::RXdgExportedV2(GXdgExporterV2 *exporterRes, UInt32 id, LToplevelRole *toplevel) noexcept
    :RXdgExportedBase
    (
        exporterRes->client()->client(),
        &zxdg_exported_v2_interface,
        exporterRes->version(),
        id,
        toplevel,
        &imp
    )
{
    zxdg_exported_v2_send_handle(resource(), handle().c_str());
}

/******************** REQUESTS ********************/

void RXdgExportedV2::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
