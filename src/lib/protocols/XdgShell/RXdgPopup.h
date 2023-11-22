#ifndef RXDGPOPUP_H
#define RXDGPOPUP_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgPopup : public LResource
{
public:
    RXdgPopup(RXdgSurface *rXdgSurface,
              RXdgSurface *rXdgParentSurface,
              RXdgPositioner *rXdgPositioner,
              UInt32 id);
    ~RXdgPopup();

    RXdgSurface *xdgSurfaceResource() const;
    LPopupRole *popupRole() const;

    // Since 1
    bool configure(Int32 x, Int32 y, Int32 width, Int32 height);
    bool popupDone();

    // Since 3
    bool repositioned(UInt32 token);

    LPRIVATE_IMP_UNIQUE(RXdgPopup)
};

#endif // RXDGPOPUP_H
