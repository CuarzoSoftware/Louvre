#ifndef RTEARINGCONTROL_H
#define RTEARINGCONTROL_H

#include <LResource.h>

class Louvre::Protocols::TearingControl::RTearingControl : public LResource
{
public:
    RTearingControl(Wayland::RSurface *rSurface, Int32 version, UInt32 id);
    ~RTearingControl();

    Wayland::RSurface *surfaceResource() const;
    bool preferVSync() const;

    LPRIVATE_IMP_UNIQUE(RTearingControl)
};

#endif // RTEARINGCONTROL_H
