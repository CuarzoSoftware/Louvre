#ifndef RXDGIMPORTEDV1_H
#define RXDGIMPORTEDV1_H

#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>

namespace CZ::Protocols::XdgForeign
{
    class GXdgImporterV1;
    class RXdgExportedBase;
}

class CZ::Protocols::XdgForeign::RXdgImportedV1 final : public RXdgImportedBase
{
public:
    RXdgImportedV1(GXdgImporterV1 *importerRes, UInt32 id, RXdgExportedBase *exported) noexcept;

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_parent_of(wl_client *client, wl_resource *resource, wl_resource *surface) noexcept;

protected:
    void sendDestroyed() noexcept override;
};

#endif // RXDGIMPORTEDV1_H
