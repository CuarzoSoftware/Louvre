#ifndef GGAMMACONTROLMANAGER_H
#define GGAMMACONTROLMANAGER_H

#include <LResource.h>

class Louvre::Protocols::GammaControl::GGammaControlManager : public LResource
{
public:
    GGammaControlManager(LClient *client,
                const wl_interface *interface,
                Int32 version,
                UInt32 id,
                const void *implementation);
    ~GGammaControlManager();

    LPRIVATE_IMP_UNIQUE(GGammaControlManager)
};

#endif // GGAMMACONTROLMANAGER_H
