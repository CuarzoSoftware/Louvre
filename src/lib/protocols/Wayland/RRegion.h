#ifndef REGIONRESOURCE_H
#define REGIONRESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RRegion : public LResource
{
public:
    RRegion(GCompositor *compositorGlobal, UInt32 id);
    ~RRegion();

    const LRegion &region() const;

    LPRIVATE_IMP(RRegion)
};

#endif // REGIONRESOURCE_H
