#ifndef XDGPOSITIONER_H
#define XDGPOSITIONER_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgPositioner : public LResource
{
public:
    RXdgPositioner(GXdgWmBase *gXdgWmBase, UInt32 id);
    ~RXdgPositioner();

    const LPositioner &positioner() const;

    LPRIVATE_IMP(RXdgPositioner)
};

#endif // XDGPOSITIONER_H
