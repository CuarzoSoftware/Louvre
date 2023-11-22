#ifndef RXDGSURFACE_H
#define RXDGSURFACE_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgSurface : public LResource
{
public:
    RXdgSurface(GXdgWmBase *gXdgWmBase, LSurface *lSurface, UInt32 id);
    ~RXdgSurface();

    GXdgWmBase *xdgWmBaseGlobal() const;
    LSurface *surface() const;
    RXdgToplevel *xdgToplevelResource() const;
    RXdgPopup *xdgPopupResource() const;

    // Since 1
    bool configure(UInt32 serial) const;

    LPRIVATE_IMP_UNIQUE(RXdgSurface)
};

#endif // RXDGSURFACE_H
