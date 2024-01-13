#ifndef RFRACTIONALSCALE_H
#define RFRACTIONALSCALE_H

#include <LResource.h>

class Louvre::Protocols::FractionalScale::RFractionalScale : public LResource
{
public:
    RFractionalScale(Wayland::RSurface *rSurface, UInt32 id, Int32 version);
    ~RFractionalScale();

    Wayland::RSurface *surfaceResource() const;

    // Since 1
    bool preferredScale(Float32 scale);

    LPRIVATE_IMP_UNIQUE(RFractionalScale)
};

#endif // RFRACTIONALSCALE_H
