#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgExporterV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedV1.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/LCompositor.h>
using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_exporter_v1_interface imp
{
    .destroy = &GXdgExporterV1::destroy,
    .export_ = &GXdgExporterV1::export_toplevel
};

LGLOBAL_INTERFACE_IMP(GXdgExporterV1, LOUVRE_XDG_FOREIGN_V1_VERSION, zxdg_exporter_v1_interface)

bool GXdgExporterV1::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgExporterV1)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgExporterV1;
    return true;
}

GXdgExporterV1::GXdgExporterV1
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

GXdgExporterV1::~GXdgExporterV1() noexcept {}

/******************** REQUESTS ********************/

void GXdgExporterV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgExporterV1::export_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto &exporter { LRES_CAST(GXdgExporterV1, resource) };

    // The v1 `export` argument is a wl_surface on the wire.
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    // v1 defines no protocol error for a surface without the xdg_toplevel role. The object must
    // still be created (the client already did): an inert one, without a handle.
    LToplevelRole *toplevel { surfaceRes->surface() ? surfaceRes->surface()->toplevel() : nullptr };

    if (!toplevel)
        LLog(CZWarning, CZLN, "zxdg_exporter_v1.export of a surface without the xdg_toplevel role: no handle sent");

    new RXdgExportedV1(&exporter, id, toplevel);
}
