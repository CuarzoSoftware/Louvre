#ifndef XDGWMBASE_H
#define XDGWMBASE_H

#include <LResource.h>

using namespace std;

class Louvre::Protocols::XdgShell::GXdgWmBase : public LResource
{
public:
    GXdgWmBase(wl_client *client,
               const wl_interface *interface,
               Int32 version,
               UInt32 id,
               const void *implementation,
               wl_resource_destroy_func_t destroy);

    ~GXdgWmBase();

    const list<RXdgSurface*> &xdgSurfaces() const;
    void ping(UInt32 serial) const;

    LPRIVATE_IMP(GXdgWmBase)
};

#endif // XDGWMBASE_H
