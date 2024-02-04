#ifndef RGAMMACONTROL_H
#define RGAMMACONTROL_H

#include <LResource.h>

class Louvre::Protocols::GammaControl::RGammaControl : public LResource
{
public:
    RGammaControl(Wayland::GOutput *gOutput, Int32 version, UInt32 id);
    ~RGammaControl();

    Wayland::GOutput *outputGlobal() const;

    // Since 1
    bool gammaSize(UInt32 size);
    bool failed();

    LPRIVATE_IMP_UNIQUE(RGammaControl)
};

#endif // RGAMMACONTROL_H
