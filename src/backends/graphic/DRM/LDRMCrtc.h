#ifndef LDRMCRTC_H
#define LDRMCRTC_H

#include <LNamespaces.h>

using namespace Louvre;

#include <xf86drm.h>
#include <xf86drmMode.h>

class LDRMDevice;

class LDRMCrtc
{
public:
    LDRMCrtc(LDRMDevice *device, drmModeCrtc *crtc);
    ~LDRMCrtc();

    UInt32 id() const;
    drmModeCrtc *crtc() const;
    LDRMDevice *device() const;
    void setDRMResouce(drmModeCrtc *crtc);
private:
    drmModeCrtc *m_crtc = nullptr;
    LDRMDevice *m_device = nullptr;
};

#endif // LDRMCRTC_H
