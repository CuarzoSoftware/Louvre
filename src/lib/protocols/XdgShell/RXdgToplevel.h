#ifndef XDGTOPLEVEL_H
#define XDGTOPLEVEL_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgToplevel : public LResource
{
public:
    RXdgToplevel(RXdgSurface *rXdgSurface, UInt32 id);
    ~RXdgToplevel();

    RXdgSurface *rXdgSurface() const;
    LToplevelRole *lToplevelRole() const;

    bool configure(Int32 width, Int32 height, wl_array *states) const;
    bool close() const;

    // Since 4
    bool configure_bounds(Int32 width, Int32 height) const;

    // Since 5
    bool wm_capabilities(wl_array *capabilities) const;

    LPRIVATE_IMP(RXdgToplevel)
};

#endif // XDGTOPLEVEL_H
