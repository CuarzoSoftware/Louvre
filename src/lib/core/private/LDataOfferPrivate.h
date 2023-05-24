#ifndef LDATAOFFERPRIVATE_H
#define LDATAOFFERPRIVATE_H

#include <LDataOffer.h>
#include <LDNDManager.h>
#include <private/LDataSourcePrivate.h>

class Louvre::LDataOffer::LDataOfferPrivate
{
public:
    LDataOfferPrivate()                                     = default;
    ~LDataOfferPrivate()                                    = default;

    LDataOfferPrivate(const LDataOfferPrivate&)             = delete;
    LDataOfferPrivate &operator=(const LDataOfferPrivate&)  = delete;

    Protocols::Wayland::RDataOffer *dataOfferResource = nullptr;
    bool hasFinished                                        = false;
    Usage usedFor                                           = Usage::Undefined;

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    UInt32 acceptedActions                                  = DND_NO_ACTION_SET;
    UInt32 preferredAction                                  = DND_NO_ACTION_SET;
    void updateDNDAction();
#endif
};


#endif // LDATAOFFERPRIVATE_H
