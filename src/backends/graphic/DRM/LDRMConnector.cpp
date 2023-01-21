#include "LDRMConnector.h"
#include "LDRMDevice.h"


LDRMConnector::LDRMConnector(LDRMDevice *device, drmModeConnector *connector)
{
    m_connector = connector;
    m_device = device;
    device->connectors().push_back(this);
}

LDRMConnector::~LDRMConnector()
{
    if(m_connector)
        drmModeFreeConnector(m_connector);

    device()->connectors().remove(this);
}

UInt32 LDRMConnector::id() const
{
    return connector()->connector_id;
}

drmModeConnector *LDRMConnector::connector() const
{
    return m_connector;
}

LDRMDevice *LDRMConnector::device() const
{
    return m_device;
}

bool LDRMConnector::connected() const
{
    return connector()->connection == DRM_MODE_CONNECTED && connector()->mmHeight != 0 && connector()->mmWidth != 0;
}

void LDRMConnector::setDRMResouce(drmModeConnector *connector)
{
    if(m_connector)
        drmModeFreeConnector(m_connector);

    m_connector = connector;
}
