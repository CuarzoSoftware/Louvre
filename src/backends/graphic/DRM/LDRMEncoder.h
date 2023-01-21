#ifndef LDRMENCODER_H
#define LDRMENCODER_H

#include "LDRMCrtc.h"
#include <LNamespaces.h>

using namespace Louvre;
using namespace std;

#include <xf86drm.h>
#include <xf86drmMode.h>

class LDRMDevice;

class LDRMEncoder
{
public:
    LDRMEncoder(LDRMDevice *device, drmModeEncoder *encoder);
    ~LDRMEncoder();

    UInt32 id() const;
    drmModeEncoder *encoder() const;
    LDRMDevice *device() const;
    const char *typeStr() const;
    void setDRMResouce(drmModeEncoder *encoder);
    list<LDRMEncoder*>&possibleClones();
    list<LDRMCrtc*>&possibleCrtcs();

    bool updatePossibleClones();
    bool updatePossibleCrtcs();
private:
    drmModeEncoder *m_encoder = nullptr;
    LDRMDevice *m_device = nullptr;
    list<LDRMEncoder*>m_possibleClones;
    list<LDRMCrtc*>m_possibleCrts;

};

#endif // LDRMENCODER_H
