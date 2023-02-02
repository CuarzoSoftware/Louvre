#include "protocols/Wayland/SeatGlobal.h"
#include <cstring>
#include <protocols/Wayland/private/DataOfferResourcePrivate.h>
#include <protocols/Wayland/private/DataSourceResourcePrivate.h>
#include <protocols/Wayland/private/DataDeviceResourcePrivate.h>

#include <LClient.h>
#include <LSeat.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <unistd.h>

void DataOfferResource::DataOfferResourcePrivate::resource_destroy(wl_resource *resource)
{
    DataOfferResource *lDataOfferResource = (DataOfferResource*)wl_resource_get_user_data(resource);

    for(Protocols::Wayland::SeatGlobal *s : lDataOfferResource->client()->seatGlobals())
    {
        if(s->dataDeviceResource() && s->dataDeviceResource()->dataOffered() == lDataOfferResource->dataOffer())
        {
            s->dataDeviceResource()->imp()->dataOffered = nullptr;
        }
    }

    delete lDataOfferResource;
}

void DataOfferResource::DataOfferResourcePrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void DataOfferResource::DataOfferResourcePrivate::accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type)
{
    L_UNUSED(client);

    DataOfferResource *lDataOfferResource = (DataOfferResource*)wl_resource_get_user_data(resource);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if(lDataOfferResource->version() >= 3 && lDataOfferResource->dataOffer()->imp()->hasFinished)
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH, "Invalid DND action mask.");
#endif

    if(mime_type != NULL)
        lDataOfferResource->client()->seat()->dndManager()->imp()->matchedMimeType = true;

}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void DataOfferResource::DataOfferResourcePrivate::finish(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    DataOfferResource *lDataOfferResource = (DataOfferResource*)wl_resource_get_user_data(resource);

    if(lDataOfferResource->dataOffer()->usedFor() != LDataOffer::DND)
    {
        wl_resource_post_error(resource,WL_DATA_OFFER_ERROR_INVALID_FINISH,"Data offer not used for DND.");
        return;
    }

    lDataOfferResource->dataOffer()->imp()->hasFinished = true;

    if(lDataOfferResource->client()->seat()->dndManager()->source() &&
            lDataOfferResource->client()->seat()->dndManager()->source()->dataSourceResource()->version() >= 3)
        wl_data_source_send_dnd_finished(lDataOfferResource->client()->seat()->dndManager()->source()->dataSourceResource()->resource());

    if(lDataOfferResource->client()->seat()->dndManager()->focus())
        lDataOfferResource->client()->seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    lDataOfferResource->client()->seat()->dndManager()->imp()->clear();

}
#endif

void DataOfferResource::DataOfferResourcePrivate::receive(wl_client *client, wl_resource *resource, const char *mime_type, Int32 fd)
{
    L_UNUSED(client);

    DataOfferResource *lDataOfferResource = (DataOfferResource*)wl_resource_get_user_data(resource);

    // If used in drag n drop
    if(lDataOfferResource->dataOffer()->usedFor() == LDataOffer::DND && lDataOfferResource->client()->seat()->dndManager()->source())
    {
        // Ask the source client to write the data to the FD given the mime type
        lDataOfferResource->client()->seat()->dndManager()->source()->dataSourceResource()->sendSend(mime_type, fd);
    }

    // If used in clipboard
    else if(lDataOfferResource->dataOffer()->usedFor() == LDataOffer::Selection && lDataOfferResource->client()->seat()->dataSelection())
    {
        // Louvre keeps a copy of the source clipboard for each mime type (so we don't ask the source client to write the data)
        for(LDataSource::LSource &s : lDataOfferResource->client()->seat()->dataSelection()->imp()->sources)
        {
            if(strcmp(s.mimeType, mime_type) == 0)
            {
                fseek(s.tmp, 0L, SEEK_END);

                // If pointer is at the beggining means the source client has not written any data
                if(ftell(s.tmp) == 0)
                    break;

                rewind(s.tmp);
                char byte = fgetc(s.tmp);

                while(!feof(s.tmp))
                {
                    write(fd, &byte, 1);
                    byte = fgetc(s.tmp);
                }

                break;
            }
        }
    }

    close(fd);
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

void DataOfferResource::DataOfferResourcePrivate::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action)
{
    DataOfferResource *lDataOfferResource = (DataOfferResource*)wl_resource_get_user_data(resource);

    if(lDataOfferResource->dataOffer()->imp()->acceptedActions == dnd_actions &&
            lDataOfferResource->dataOffer()->imp()->preferredAction == preferred_action)
        return;

    lDataOfferResource->dataOffer()->imp()->acceptedActions = dnd_actions;
    lDataOfferResource->dataOffer()->imp()->preferredAction = preferred_action;
    lDataOfferResource->dataOffer()->imp()->updateDNDAction();
}
#endif
