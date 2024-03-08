#ifndef LDNDSESSION_H
#define LDNDSESSION_H

#include <LSurface.h>
#include <LDNDIconRole.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/RDataDevice.h>

using namespace Louvre::Protocols::Wayland;

class Louvre::LDNDSession : public LObject
{
public:
    UInt32 compositorAction { 0 };
    bool matchedMimeType { false };
    bool dropped { false };
    LWeak<LSurface> focus;
    LWeak<LSurface> origin;
    LWeak<LDNDIconRole> icon;
    LWeak<RDataDevice> srcDataDevice;
    LWeak<RDataDevice> dstDataDevice;
    LWeak<RDataSource> source;
    LWeak<RDataOffer> offer;

    void updateActions()
    {

    }
};

#endif // LDNDSESSION_H
