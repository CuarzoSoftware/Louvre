#ifndef RXDGTOPLEVEL_H
#define RXDGTOPLEVEL_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgToplevel : public LResource
{
public:
    RXdgToplevel(RXdgSurface *rXdgSurface, UInt32 id);
    ~RXdgToplevel();

    RXdgSurface *xdgSurfaceResource() const;
    LToplevelRole *toplevelRole() const;

    // Since 1
    bool configure(Int32 width, Int32 height, wl_array *states);
    bool close();

    // Since 4
    bool configureBounds(Int32 width, Int32 height);

    // Since 5
    bool wmCapabilities(wl_array *capabilities);

    LPRIVATE_IMP_UNIQUE(RXdgToplevel)
};

#endif // RXDGTOPLEVEL_H
