#ifndef XDGTOPLEVELDECORATION_H
#define XDGTOPLEVELDECORATION_H

#include <LResource.h>

class Louvre::Protocols::XdgDecoration::RXdgToplevelDecoration : public LResource
{
public:
    RXdgToplevelDecoration(GXdgDecorationManager *gXdgDecorationManager,
                           LToplevelRole *lToplevelRole,
                           UInt32 id);
    ~RXdgToplevelDecoration();

    LToplevelRole *lToplevelRole() const;

    bool configure(UInt32 mode) const;

    LPRIVATE_IMP(RXdgToplevelDecoration)
};

#endif // XDGTOPLEVELDECORATION_H
