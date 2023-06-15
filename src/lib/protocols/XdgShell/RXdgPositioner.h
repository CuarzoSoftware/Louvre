#ifndef RXDGPOSITIONER_H
#define RXDGPOSITIONER_H

#include <LResource.h>

class Louvre::Protocols::XdgShell::RXdgPositioner : public LResource
{
public:
    RXdgPositioner(GXdgWmBase *gXdgWmBase, UInt32 id);
    ~RXdgPositioner();

    const LPositioner &positioner() const;
    bool isValid();

    LPRIVATE_IMP(RXdgPositioner)
};

#endif // RXDGPOSITIONER_H
