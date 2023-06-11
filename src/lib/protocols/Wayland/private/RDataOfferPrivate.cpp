#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <LClient.h>
#include <LSeat.h>
#include <unistd.h>
#include <cstring>

void RDataOffer::RDataOfferPrivate::resource_destroy(wl_resource *resource)
{
    RDataOffer *rDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    for (GSeat *s : rDataOffer->client()->seatGlobals())
        if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered() == rDataOffer->dataOffer())
            s->dataDeviceResource()->imp()->dataOffered = nullptr;

    delete rDataOffer;
}

void RDataOffer::RDataOfferPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RDataOffer::RDataOfferPrivate::accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type)
{
    L_UNUSED(client);
    L_UNUSED(serial);

    /* TODO: Use serial */
    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (lRDataOffer->version() >= 3 && lRDataOffer->dataOffer()->imp()->hasFinished)
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH, "Invalid DND action mask.");
#endif

    if (mime_type != NULL)
        seat()->dndManager()->imp()->matchedMimeType = true;
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::RDataOfferPrivate::finish(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    if (lRDataOffer->dataOffer()->usedFor() != LDataOffer::DND)
    {
        wl_resource_post_error(resource,WL_DATA_OFFER_ERROR_INVALID_FINISH,"Data offer not used for DND.");
        return;
    }

    lRDataOffer->dataOffer()->imp()->hasFinished = true;

    if (seat()->dndManager()->source() &&
            seat()->dndManager()->source()->dataSourceResource()->version() >= 3)
        wl_data_source_send_dnd_finished(seat()->dndManager()->source()->dataSourceResource()->resource());

    if (seat()->dndManager()->focus())
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    seat()->dndManager()->imp()->clear();
}
#endif

void RDataOffer::RDataOfferPrivate::receive(wl_client *client, wl_resource *resource, const char *mime_type, Int32 fd)
{
    L_UNUSED(client);

    RDataOffer *rDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    // If used in drag n drop
    if (rDataOffer->dataOffer()->usedFor() == LDataOffer::DND && seat()->dndManager()->source())
    {
        // Ask the source client to write the data to the FD given the mime type
        seat()->dndManager()->source()->dataSourceResource()->send(mime_type, fd);
    }

    // If used in clipboard
    else if (rDataOffer->dataOffer()->usedFor() == LDataOffer::Selection && seat()->dataSelection())
    {
        // Louvre keeps a copy of the source clipboard for each mime type (so we don't ask the source client to write the data)
        for (LDataSource::LSource &s : seat()->dataSelection()->imp()->sources)
        {
            if (strcmp(s.mimeType, mime_type) == 0)
            {
                fseek(s.tmp, 0L, SEEK_END);

                // If pointer is at the beggining means the source client has not written any data
                if (ftell(s.tmp) == 0)
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

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::RDataOfferPrivate::set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action)
{
    L_UNUSED(client);

    RDataOffer *rDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    if (rDataOffer->dataOffer()->imp()->acceptedActions == dnd_actions && rDataOffer->dataOffer()->imp()->preferredAction == preferred_action)
        return;

    if (rDataOffer->dataOffer()->usedFor() != LDataOffer::DND)
    {
        wl_resource_post_error(resource, -1, "Data offer not being used for DND.");
        return;
    }

    if (dnd_actions > 8)
    {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION, "Invalid dnd_actions.");
        return;
    }

    if (preferred_action != 0 && preferred_action != 1 && preferred_action != 2 && preferred_action != 4)
    {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK, "Invalid preferred_action.");
        return;
    }

    rDataOffer->dataOffer()->imp()->acceptedActions = dnd_actions;
    rDataOffer->dataOffer()->imp()->preferredAction = preferred_action;
    rDataOffer->dataOffer()->imp()->updateDNDAction();
}
#endif
