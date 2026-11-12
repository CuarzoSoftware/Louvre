#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v2.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgImporterV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedV2.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedBase.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/LCompositor.h>
using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_importer_v2_interface imp
{
    .destroy = &GXdgImporterV2::destroy,
    .import_toplevel = &GXdgImporterV2::import_toplevel
};

LGLOBAL_INTERFACE_IMP(GXdgImporterV2, LOUVRE_XDG_FOREIGN_V2_VERSION, zxdg_importer_v2_interface)

bool GXdgImporterV2::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgImporterV2)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgImporterV2;
    return true;
}

GXdgImporterV2::GXdgImporterV2
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

GXdgImporterV2::~GXdgImporterV2() noexcept {}

/******************** REQUESTS ********************/

void GXdgImporterV2::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgImporterV2::import_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id, const char *handle) noexcept
{
    auto &importer { LRES_CAST(GXdgImporterV2, resource) };

    // An invalid handle (unknown, or its toplevel is gone) gets the destroyed event right away.
    new RXdgImportedV2(&importer, id, RXdgExportedBase::Find(handle));
}
