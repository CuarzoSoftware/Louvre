#ifndef GCOMPOSITOR_H
#define GCOMPOSITOR_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GCompositor : public LResource
{
public:
    GCompositor(LClient *client,
                const wl_interface *interface,
                Int32 version,
                UInt32 id,
                const void *implementation);
    ~GCompositor();

    LPRIVATE_IMP_UNIQUE(GCompositor)
};

#endif // GCOMPOSITOR_H
