#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v2.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgImporterV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_imported_v2_interface imp
{
    .destroy = &RXdgImportedV2::destroy,
    .set_parent_of = &RXdgImportedV2::set_parent_of
};

RXdgImportedV2::RXdgImportedV2(GXdgImporterV2 *importerRes, UInt32 id, RXdgExportedBase *exported) noexcept
    :RXdgImportedBase
    (
        importerRes->client()->client(),
        &zxdg_imported_v2_interface,
        importerRes->version(),
        id,
        exported,
        &imp
    )
{}

/******************** REQUESTS ********************/

void RXdgImportedV2::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RXdgImportedV2::set_parent_of(wl_client */*client*/, wl_resource *resource, wl_resource *surface) noexcept
{
    auto &imported { LRES_CAST(RXdgImportedV2, resource) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    // v2 mandates the invalid_surface error for a non-toplevel child.
    if (!surfaceRes->surface() || !surfaceRes->surface()->toplevel())
    {
        imported.postError(ZXDG_IMPORTED_V2_ERROR_INVALID_SURFACE,
                           "The given wl_surface does not have the xdg_toplevel role assigned.");
        return;
    }

    imported.setParentOf(surfaceRes->surface()->toplevel());
}

/******************** EVENTS ********************/

void RXdgImportedV2::sendDestroyed() noexcept
{
    zxdg_imported_v2_send_destroyed(resource());
}
