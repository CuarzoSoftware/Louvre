#include <protocols/Wayland/DataSourceResource.h>
#include <protocols/Wayland/DataOfferResource.h>

#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataSourcePrivate.h>

#include <LClient.h>
#include <LSeat.h>

using namespace Louvre;

LDataOffer::LDataOffer(Protocols::Wayland::DataOfferResource *dataOfferResource)
{
    m_imp = new LDataOfferPrivate();
    imp()->dataOfferResource = dataOfferResource;
}

LDataOffer::~LDataOffer()
{
    delete m_imp;
}

LSeat *LDataOffer::seat() const
{
    return dataOfferResource()->client()->seat();
}

Protocols::Wayland::DataOfferResource *LDataOffer::dataOfferResource() const
{
    return imp()->dataOfferResource;
}

LDataOffer::Usage LDataOffer::usedFor() const
{
    return imp()->usedFor;
}

LDataOffer::LDataOfferPrivate *LDataOffer::imp() const
{
    return m_imp;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

void LDataOffer::LDataOfferPrivate::updateDNDAction()
{
    LDNDManager *dndManager = dataOfferResource->client()->seat()->dndManager();

    bool offerIsV3 = dataOfferResource->version() >= 3;
    UInt32 compositorAction = dndManager->preferredAction();
    UInt32 final = 0;

    // If has source
    if(dndManager->source())
    {
        bool sourceIsV3 = dndManager->source()->dataSourceResource()->version() >= 3;

        // If both are v3
        if(sourceIsV3 && offerIsV3)
        {
            // If offer has not sent preferred action
            if(preferredAction == DND_NO_ACTION_SET)
            {
                caseA:
                UInt32 both = dndManager->source()->dndActions();

                final = both;

                if(compositorAction != LDNDManager::NoAction)
                    final = both & compositorAction;

                if(final & LDNDManager::Copy)
                    final = LDNDManager::Copy;

                else if(final & LDNDManager::Move)
                    final = LDNDManager::Move;

                else if(final & LDNDManager::Ask)
                    final = LDNDManager::Ask;

                else
                    final = LDNDManager::NoAction;

                dndManager->source()->dataSourceResource()->sendAction(final);
            }
            // Offer has set action
            else
            {
                UInt32 both = dndManager->source()->dndActions() & acceptedActions;

                final = both;

                if(compositorAction != LDNDManager::NoAction)
                    final = both & compositorAction;

                if(final & preferredAction)
                    final = preferredAction;

                else if(final & LDNDManager::Copy)
                    final = LDNDManager::Copy;

                else if(final & LDNDManager::Move)
                    final = LDNDManager::Move;

                else if(final & LDNDManager::Ask)
                    final = LDNDManager::Ask;

                else
                    final = LDNDManager::NoAction;

                dataOfferResource->sendAction(final);
                dndManager->source()->dataSourceResource()->sendAction(final);
            }

        }
        else if(sourceIsV3 && !offerIsV3)
        {
            goto caseA;
        }
        else if(!sourceIsV3 && offerIsV3)
        {
            if(preferredAction != DND_NO_ACTION_SET)
            {
                UInt32 both = acceptedActions;

                final = both;

                if(compositorAction != LDNDManager::NoAction)
                    final = both & compositorAction;

                if(final & preferredAction)
                    final = preferredAction;

                else if(final & LDNDManager::Copy)
                    final = LDNDManager::Copy;

                else if(final & LDNDManager::Move)
                    final = LDNDManager::Move;

                else if(final & LDNDManager::Ask)
                    final = LDNDManager::Ask;
                else
                    final = LDNDManager::NoAction;

                dataOfferResource->sendAction(final);
            }
        }

    }
    // If no source
    else
    {
        if(offerIsV3 && preferredAction != DND_NO_ACTION_SET)
        {
            UInt32 both = acceptedActions;

            final = both;

            if(compositorAction != LDNDManager::NoAction)
                final = both & compositorAction;

            if(final & preferredAction)
                final = preferredAction;

            else if(final & LDNDManager::Copy)
                final = LDNDManager::Copy;

            else if(final & LDNDManager::Move)
                final = LDNDManager::Move;

            else if(final & LDNDManager::Ask)
                final = LDNDManager::Ask;
            else
                final = LDNDManager::NoAction;

            dataOfferResource->sendAction(final);
        }
    }

}

#endif
