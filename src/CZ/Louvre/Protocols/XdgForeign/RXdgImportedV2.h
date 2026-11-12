#ifndef RXDGIMPORTEDV2_H
#define RXDGIMPORTEDV2_H

#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>

namespace CZ::Protocols::XdgForeign
{
    class GXdgImporterV2;
    class RXdgExportedBase;
}

class CZ::Protocols::XdgForeign::RXdgImportedV2 final : public RXdgImportedBase
{
public:
    RXdgImportedV2(GXdgImporterV2 *importerRes, UInt32 id, RXdgExportedBase *exported) noexcept;

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_parent_of(wl_client *client, wl_resource *resource, wl_resource *surface) noexcept;

protected:
    void sendDestroyed() noexcept override;
};

#endif // RXDGIMPORTEDV2_H
