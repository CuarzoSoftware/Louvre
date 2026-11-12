#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgImporterV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_imported_v1_interface imp
{
    .destroy = &RXdgImportedV1::destroy,
    .set_parent_of = &RXdgImportedV1::set_parent_of
};

RXdgImportedV1::RXdgImportedV1(GXdgImporterV1 *importerRes, UInt32 id, RXdgExportedBase *exported) noexcept
    :RXdgImportedBase
    (
        importerRes->client()->client(),
        &zxdg_imported_v1_interface,
        importerRes->version(),
        id,
        exported,
        &imp
    )
{}

/******************** REQUESTS ********************/

void RXdgImportedV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RXdgImportedV1::set_parent_of(wl_client */*client*/, wl_resource *resource, wl_resource *surface) noexcept
{
    auto &imported { LRES_CAST(RXdgImportedV1, resource) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    // A surface without the xdg_toplevel role is ignored (the importer
    // interface defines no protocol error for it).
    imported.setParentOf(surfaceRes->surface() ? surfaceRes->surface()->toplevel() : nullptr);
}

/******************** EVENTS ********************/

void RXdgImportedV1::sendDestroyed() noexcept
{
    zxdg_imported_v1_send_destroyed(resource());
}
