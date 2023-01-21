#include "LDRMCrtc.h"
#include "LDRMDevice.h"

LDRMCrtc::LDRMCrtc(LDRMDevice *device, drmModeCrtc *crtc)
{
    m_crtc = crtc;
    m_device = device;
    device->crtcs().push_back(this);
}

LDRMCrtc::~LDRMCrtc()
{
    if(m_crtc)
        drmModeFreeCrtc(m_crtc);

    device()->crtcs().remove(this);
}

UInt32 LDRMCrtc::id() const
{
    return crtc()->crtc_id;
}

drmModeCrtc *LDRMCrtc::crtc() const
{
    return m_crtc;
}

LDRMDevice *LDRMCrtc::device() const
{
    return m_device;
}

void LDRMCrtc::setDRMResouce(drmModeCrtc *crtc)
{
    if(m_crtc)
        drmModeFreeCrtc(m_crtc);

    m_crtc = crtc;
}
