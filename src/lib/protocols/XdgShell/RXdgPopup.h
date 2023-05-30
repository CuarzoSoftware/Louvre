#ifndef XDGPOPUP_H
#define XDGPOPUP_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgPopup : public LResource
{
public:
    RXdgPopup(RXdgSurface *rXdgSurface,
              RXdgSurface *rXdgParentSurface,
              RXdgPositioner *rXdgPositioner,
              UInt32 id);

    ~RXdgPopup();

    RXdgSurface *rXdgSurface() const;
    LPopupRole *lPopupRole() const;

    bool configure(Int32 x, Int32 y, Int32 width, Int32 height) const;
    bool popup_done() const;

    // Since 3
    bool repositioned(UInt32 token) const;

    LPRIVATE_IMP(RXdgPopup)
};

#endif // XDGPOPUP_H
