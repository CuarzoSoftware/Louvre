#ifndef LDRMPLANE_H
#define LDRMPLANE_H

#include "LDRMDevice.h"
#include <xf86drmMode.h>

using namespace std;

class LDRMPlane
{
public:

    struct LDRMInFormat
    {
        UInt64 format;
        UInt64 mod;
    };

    LDRMPlane(LDRMDevice *device, drmModePlane *plane);
    ~LDRMPlane();

    bool updateProperties();
    bool updatePossibleCrtcs();

    void setDRMResource(drmModePlane *plane);

    Int32 type() const;
    UInt32 id() const;
    LDRMDevice *device() const;
    drmModePlane *plane() const;

    list<LDRMCrtc*>&possibleCrtcs();
    list<LDRMInFormat>&inFormats();

private:
    LDRMDevice *m_device = nullptr;
    drmModePlane *m_plane = nullptr;
    Int32 m_type = -1;

    list<LDRMCrtc *>m_possibleCrtcs;
    list<LDRMInFormat>m_inFormats;

};

#endif // LDRMPLANE_H
