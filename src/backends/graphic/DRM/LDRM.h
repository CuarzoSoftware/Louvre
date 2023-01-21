#ifndef LDRM_H
#define LDRM_H

#include "LDRMDevice.h"
#include <LCompositor.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <LLog.h>

#define LBACKEND_NAME "\x1B[34mLGraphicBackendDRM:\x1B[0m"

using namespace std;

class LDRM
{
public:

    enum LDRMState
    {
        Uninitialized,
        Initialized
    };

    LDRM(LCompositor *compositor);
    ~LDRM();

    bool initialize();
    void updateDevices();
    void destroyDevices();
    LDRMDevice *findDevice(const char *name);

    LDRMState state() const;
    LCompositor *compositor() const;
    udev *udevHandle() const;
    list<LDRMDevice*> &devices();
    list<LOutput*> &outputs();

private:
    LDRMState m_state           = Uninitialized;
    LCompositor *m_compositor   = nullptr;
    udev *m_udev                = nullptr;

    list<LDRMDevice*> m_devices;
    list<LOutput*> m_outputs;

};

#endif // LDRM_H
