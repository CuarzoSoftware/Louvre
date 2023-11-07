#ifndef LDATAOFFERPRIVATE_H
#define LDATAOFFERPRIVATE_H

#include <LDataOffer.h>
#include <LDNDManager.h>
#include <private/LDataSourcePrivate.h>

using namespace Louvre;

LPRIVATE_CLASS(LDataOffer)
    Protocols::Wayland::RDataOffer *dataOfferResource = nullptr;
    bool hasFinished = false;
    Usage usedFor = Usage::Undefined;

    // Since 3
    UInt32 acceptedActions = LOUVRE_DND_NO_ACTION_SET;
    UInt32 preferredAction = LOUVRE_DND_NO_ACTION_SET;
    void updateDNDAction();
};

#endif // LDATAOFFERPRIVATE_H
