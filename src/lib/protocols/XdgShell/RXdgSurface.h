#ifndef XDGSURFACE_H
#define XDGSURFACE_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgSurface : public LResource
{
public:
    RXdgSurface(GXdgWmBase *gXdgWmBase, LSurface *lSurface, UInt32 id);
    ~RXdgSurface();

    GXdgWmBase *gXdgWmBase() const;
    LSurface *lSurface() const;
    RXdgToplevel *rXdgToplevel() const;
    RXdgPopup *rXdgPopup() const;

    void configure(UInt32 serial) const;

    LPRIVATE_IMP(RXdgSurface)
};

#endif // XDGSURFACE_H
