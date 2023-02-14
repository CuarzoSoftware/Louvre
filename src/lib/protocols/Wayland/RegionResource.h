#ifndef REGIONRESOURCE_H
#define REGIONRESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RegionResource : public LResource
{
public:
    RegionResource(CompositorGlobal *compositorGlobal, UInt32 id);
    ~RegionResource();

    const LRegion &region() const;

    LPRIVATE_IMP(RegionResource)
};

#endif // REGIONRESOURCE_H
