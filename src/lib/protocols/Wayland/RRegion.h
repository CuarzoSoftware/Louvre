#ifndef RREGION_H
#define RREGION_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RRegion : public LResource
{
public:
    RRegion(GCompositor *gCompositor, UInt32 id);
    ~RRegion();

    const LRegion &region() const;

    LPRIVATE_IMP_UNIQUE(RRegion)
};

#endif // RREGION_H
