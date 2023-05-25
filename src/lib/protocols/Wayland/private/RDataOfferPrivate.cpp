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
    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    for (Protocols::Wayland::GSeat *s : lRDataOffer->client()->seatGlobals())
        if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered() == lRDataOffer->dataOffer())
            s->dataDeviceResource()->imp()->dataOffered = nullptr;

    delete lRDataOffer;
}

void RDataOffer::RDataOfferPrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RDataOffer::RDataOfferPrivate::accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type)
{
    L_UNUSED(client);
    L_UNUSED(serial);

    /* TODO: Use serial */
    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if (lRDataOffer->version() >= 3 && lRDataOffer->dataOffer()->imp()->hasFinished)
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH, "Invalid DND action mask.");
#endif

    if (mime_type != NULL)
        lRDataOffer->client()->seat()->dndManager()->imp()->matchedMimeType = true;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
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

    if (lRDataOffer->client()->seat()->dndManager()->source() &&
            lRDataOffer->client()->seat()->dndManager()->source()->dataSourceResource()->version() >= 3)
        wl_data_source_send_dnd_finished(lRDataOffer->client()->seat()->dndManager()->source()->dataSourceResource()->resource());

    if (lRDataOffer->client()->seat()->dndManager()->focus())
        lRDataOffer->client()->seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    lRDataOffer->client()->seat()->dndManager()->imp()->clear();
}
#endif

void RDataOffer::RDataOfferPrivate::receive(wl_client *client, wl_resource *resource, const char *mime_type, Int32 fd)
{
    L_UNUSED(client);

    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    // If used in drag n drop
    if (lRDataOffer->dataOffer()->usedFor() == LDataOffer::DND && lRDataOffer->client()->seat()->dndManager()->source())
    {
        // Ask the source client to write the data to the FD given the mime type
        lRDataOffer->client()->seat()->dndManager()->source()->dataSourceResource()->sendSend(mime_type, fd);
    }

    // If used in clipboard
    else if (lRDataOffer->dataOffer()->usedFor() == LDataOffer::Selection && lRDataOffer->client()->seat()->dataSelection())
    {
        // Louvre keeps a copy of the source clipboard for each mime type (so we don't ask the source client to write the data)
        for (LDataSource::LSource &s : lRDataOffer->client()->seat()->dataSelection()->imp()->sources)
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

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::RDataOfferPrivate::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action)
{
    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    if (lRDataOffer->dataOffer()->imp()->acceptedActions == dnd_actions &&
            lRDataOffer->dataOffer()->imp()->preferredAction == preferred_action)
        return;

    lRDataOffer->dataOffer()->imp()->acceptedActions = dnd_actions;
    lRDataOffer->dataOffer()->imp()->preferredAction = preferred_action;
    lRDataOffer->dataOffer()->imp()->updateDNDAction();
}
#endif
