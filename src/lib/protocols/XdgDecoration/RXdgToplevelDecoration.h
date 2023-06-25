#ifndef RXDGTOPLEVELDECORATION_H
#define RXDGTOPLEVELDECORATION_H

#include <LResource.h>

class Louvre::Protocols::XdgDecoration::RXdgToplevelDecoration : public LResource
{
public:
    RXdgToplevelDecoration(GXdgDecorationManager *gXdgDecorationManager,
                           LToplevelRole *lToplevelRole,
                           UInt32 id);

    ~RXdgToplevelDecoration();

    LToplevelRole *toplevelRole() const;

    // Since 1
    bool configure(UInt32 mode);

    LPRIVATE_IMP(RXdgToplevelDecoration)
};

#endif // RXDGTOPLEVELDECORATION_H
