#ifndef RXDGEXPORTEDV2_H
#define RXDGEXPORTEDV2_H

#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>

namespace CZ
{
    class LToplevelRole;
}

namespace CZ::Protocols::XdgForeign
{
    class GXdgExporterV2;
}

class CZ::Protocols::XdgForeign::RXdgExportedV2 final : public RXdgExportedBase
{
public:

    // Sends the `handle` event: the handle as a plain string (same wire
    // shape as v1).
    RXdgExportedV2(GXdgExporterV2 *exporterRes, UInt32 id, LToplevelRole *toplevel) noexcept;

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
};

#endif // RXDGEXPORTEDV2_H
