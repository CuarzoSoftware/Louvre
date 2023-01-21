#ifndef LDRMDEVICE_H
#define LDRMDEVICE_H

class LDRM;
class LDRMPlane;
class LDRMCrtc;
class LDRMEncoder;
class LDRMConnector;
struct udev_device;

#include <LNamespaces.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

using namespace Louvre;
using namespace std;

class LDRMDevice
{
public:
    LDRMDevice(LDRM *drm, udev_device *device);
    ~LDRMDevice();

    bool update();
    bool updateDriverInfo();
    bool updateResources();
    bool updateCrtcs();
    bool updateEncoders();
    bool updateConnectors();
    bool updatePlanes();


    int id() const;
    int fd() const;
    bool valid() const;
    const char *name() const;
    LDRM *drm() const;
    udev_device *udevDevice() const;

    // Resources
    drmModeRes *resources() const;
    list<LDRMPlane*> &planes();
    list<LDRMCrtc*> &crtcs();
    list<LDRMEncoder*> &encoders();
    list<LDRMConnector*> &connectors();

    LDRMCrtc *findCrtc(UInt32 id);
    LDRMEncoder *findEncoder(UInt32 id);
    LDRMPlane *findPlane(UInt32 id);
    LDRMConnector *findConnector(UInt32 id);

    // Caps
    bool capDumbBuffer() const;
    bool capAddFB2Mod() const;
    bool capPrimeImport() const;
    bool capPrimeExport() const;

private:
    char m_name[64];
    bool m_valid                    = false;
    int m_fd                        = -1;
    int m_id                        = -1;
    LDRM *m_drm                     = nullptr;
    udev_device *m_udevDevice       = nullptr;
    drmModeRes *m_resources         = nullptr;

    // Resources
    list<LDRMPlane*>m_planes;
    list<LDRMCrtc*>m_crtcs;
    list<LDRMEncoder*>m_encoders;
    list<LDRMConnector*>m_connectors;

    // Caps
    bool m_capDumbBuffer            = false;
    bool m_capAddFB2Mod             = false;
    bool m_capPrimeImport           = false;
    bool m_capPrimeExport           = false;
};

#endif // LDRMDEVICE_H
