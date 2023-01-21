#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataSourcePrivate.h>

#include <LSeat.h>

using namespace Louvre;

LDataOffer::LDataOffer(wl_resource *resource, LDataDevice *dataDevice)
{
    m_imp = new LDataOfferPrivate();
    m_imp->resource = resource;
    m_imp->dataDevice = dataDevice;
    m_imp->seat = dataDevice->seat();
}

LDataOffer::~LDataOffer()
{
    delete m_imp;
}

wl_resource *LDataOffer::resource() const
{
    return m_imp->resource;
}

LDataDevice *LDataOffer::dataDevice() const
{
    return m_imp->dataDevice;
}

LSeat *LDataOffer::seat() const
{
    return m_imp->seat;
}

LDataOffer::Usage LDataOffer::usedFor() const
{
    return m_imp->usedFor;
}

LDataOffer::LDataOfferPrivate *LDataOffer::imp() const
{
    return m_imp;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

void LDataOffer::LDataOfferPrivate::updateDNDAction()
{
    bool offerIsV3 = wl_resource_get_version(resource) >= 3;
    UInt32 compositorAction = dataDevice->seat()->dndManager()->preferredAction();
    UInt32 final = 0;

    // If has source
    if(dataDevice->seat()->dndManager()->source())
    {
        bool sourceIsV3 = wl_resource_get_version(dataDevice->seat()->dndManager()->source()->resource()) >= 3;

        // If both are v3
        if(sourceIsV3 && offerIsV3)
        {
            // If offer has not sent preferred action
            if(preferredAction == DND_NO_ACTION_SET)
            {
                caseA:
                UInt32 both = dataDevice->seat()->dndManager()->source()->dndActions();

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

                wl_data_source_send_action(dataDevice->seat()->dndManager()->source()->resource(),final);
            }
            // Offer has set action
            else
            {
                UInt32 both = dataDevice->seat()->dndManager()->source()->dndActions() & acceptedActions;

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

                wl_data_offer_send_action(resource,final);
                wl_data_source_send_action(dataDevice->seat()->dndManager()->source()->resource(),final);
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

                wl_data_offer_send_action(resource,final);
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

            wl_data_offer_send_action(resource,final);
        }
    }

}

#endif
