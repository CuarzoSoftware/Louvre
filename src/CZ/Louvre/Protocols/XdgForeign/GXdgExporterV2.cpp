#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v2.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgExporterV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedV2.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LLog.h>
using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_exporter_v2_interface imp
{
    .destroy = &GXdgExporterV2::destroy,
    .export_toplevel = &GXdgExporterV2::export_toplevel
};

LGLOBAL_INTERFACE_IMP(GXdgExporterV2, LOUVRE_XDG_FOREIGN_V2_VERSION, zxdg_exporter_v2_interface)

bool GXdgExporterV2::Probe(CZWeak<LGlobal> **/*slot*/) noexcept
{
    return true;
}

GXdgExporterV2::GXdgExporterV2
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{}

GXdgExporterV2::~GXdgExporterV2() noexcept {}

/******************** REQUESTS ********************/

void GXdgExporterV2::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgExporterV2::export_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto &exporter { LRES_CAST(GXdgExporterV2, resource) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (!surfaceRes->surface() || !surfaceRes->surface()->toplevel())
    {
        surfaceRes->postError(ZXDG_EXPORTER_V2_ERROR_INVALID_SURFACE,
                              "The given wl_surface does not have the xdg_toplevel role assigned.");
        return;
    }

    new RXdgExportedV2(&exporter, id, surfaceRes->surface()->toplevel());
}
