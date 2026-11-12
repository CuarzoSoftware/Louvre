#ifndef RXDGEXPORTEDV1_H
#define RXDGEXPORTEDV1_H

#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>

namespace CZ
{
    class LToplevelRole;
}

namespace CZ::Protocols::XdgForeign
{
    class GXdgExporterV1;
}

class CZ::Protocols::XdgForeign::RXdgExportedV1 final : public RXdgExportedBase
{
public:

    // Sends the `handle` event: the handle string directly (v1).
    RXdgExportedV1(GXdgExporterV1 *exporterRes, UInt32 id, LToplevelRole *toplevel) noexcept;

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
};

#endif // RXDGEXPORTEDV1_H
