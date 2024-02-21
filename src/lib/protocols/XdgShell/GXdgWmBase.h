#ifndef GXDGWMBASE_H
#define GXDGWMBASE_H

#include <LResource.h>

using namespace std;

class Louvre::Protocols::XdgShell::GXdgWmBase : public LResource
{
public:
    GXdgWmBase(wl_client *client,
               const wl_interface *interface,
               Int32 version,
               UInt32 id,
               const void *implementation);

    ~GXdgWmBase();

    const std::vector<RXdgSurface*> &xdgSurfaces() const;

    // Since 1
    bool ping(UInt32 serial);

    LPRIVATE_IMP_UNIQUE(GXdgWmBase)
};

#endif // GXDGWMBASE_H
