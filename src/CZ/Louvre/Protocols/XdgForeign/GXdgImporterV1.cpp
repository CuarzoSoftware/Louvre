#include <CZ/Louvre/Protocols/XdgForeign/xdg-foreign-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgForeign/GXdgImporterV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedV1.h>
#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

static const struct zxdg_importer_v1_interface imp
{
    .destroy = &GXdgImporterV1::destroy,
    .import = &GXdgImporterV1::import_toplevel
};

LGLOBAL_INTERFACE_IMP(GXdgImporterV1, LOUVRE_XDG_FOREIGN_V1_VERSION, zxdg_importer_v1_interface)

bool GXdgImporterV1::Probe(CZWeak<LGlobal> **/*slot*/) noexcept
{
    return true;
}

GXdgImporterV1::GXdgImporterV1
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

GXdgImporterV1::~GXdgImporterV1() noexcept {}

/******************** REQUESTS ********************/

void GXdgImporterV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgImporterV1::import_toplevel(wl_client */*client*/, wl_resource *resource, UInt32 id, const char *handle) noexcept
{
    auto &importer { LRES_CAST(GXdgImporterV1, resource) };

    // An unknown handle creates an inert imported object (per spec).
    new RXdgImportedV1(&importer, id, XdgForeignRegistryInternal::FindExported(handle));
}
