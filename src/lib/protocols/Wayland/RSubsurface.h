#ifndef SUBSURFACE_H
#define SUBSURFACE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RSubsurface : public LResource
{
public:
    RSubsurface(GSubcompositor *subcompositor,
                LSurface *surface,
                LSurface *parent,
                UInt32 id);
    ~RSubsurface();

    LSubsurfaceRole *subsurfaceRole() const;
    LPRIVATE_IMP(RSubsurface)
};

#endif // SUBSURFACE_H
