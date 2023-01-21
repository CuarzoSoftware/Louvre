#ifndef LDRMCONNECTOR_H
#define LDRMCONNECTOR_H

#include <LNamespaces.h>

using namespace Louvre;

#include <xf86drm.h>
#include <xf86drmMode.h>

class LDRMDevice;


class LDRMConnector
{
public:
    LDRMConnector(LDRMDevice *device, drmModeConnector *connector);
    ~LDRMConnector();

    UInt32 id() const;
    drmModeConnector *connector() const;
    LDRMDevice *device() const;
    bool connected() const;
    void setDRMResouce(drmModeConnector *connector);
private:
    drmModeConnector *m_connector = nullptr;
    LDRMDevice *m_device = nullptr;

};

#endif // LDRMCONNECTOR_H
