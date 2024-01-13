#ifndef RVIEWPORT_H
#define RVIEWPORT_H

#include <LResource.h>

class Louvre::Protocols::Viewporter::RViewport : public LResource
{
public:
    RViewport(Wayland::RSurface *rSurface, Int32 version, UInt32 id);
    ~RViewport();

    Wayland::RSurface *surfaceResource() const;
    const LSize &dstSize() const;
    const LRectF &srcRect() const;

    LPRIVATE_IMP_UNIQUE(RViewport)
};

#endif // RVIEWPORT_H
