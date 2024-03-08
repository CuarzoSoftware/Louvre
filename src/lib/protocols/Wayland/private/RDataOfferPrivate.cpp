#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <LClipboard.h>
#include <LClient.h>
#include <LSeat.h>
#include <LDND.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

void RDataOffer::RDataOfferPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RDataOffer::RDataOfferPrivate::accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type)
{
    /* TODO DND ONLY
    L_UNUSED(client);
    L_UNUSED(serial);

    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    if (!seat()->dnd()->focus())
        return;

    if (seat()->dnd()->focus()->client() != lRDataOffer->client())
        return;

    if (seat()->dnd()->source())
    {
        seat()->dnd()->source()->dataSourceResource()->target(mime_type);
        seat()->dnd()->imp()->matchedMimeType = mime_type != NULL;
    }
    */
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::RDataOfferPrivate::finish(wl_client *client, wl_resource *resource)
{
    /* TODO DND ONLY
    L_UNUSED(client);

    RDataOffer *lRDataOffer = (RDataOffer*)wl_resource_get_user_data(resource);

    if (lRDataOffer->dataOffer()->usedFor() != LDataOffer::DND)
    {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH, "Data offer not used for DND.");
        return;
    }

    if (!seat()->dndManager()->focus())
        return;

    if (seat()->dndManager()->focus()->client() != lRDataOffer->client())
        return;

    lRDataOffer->dataOffer()->imp()->hasFinished = true;

    if (seat()->dndManager()->source())
        seat()->dndManager()->source()->dataSourceResource()->dndFinished();

    if (seat()->dndManager()->focus())
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    seat()->dndManager()->imp()->clear();
    */
}
#endif

void RDataOffer::RDataOfferPrivate::receive(wl_client *client, wl_resource *resource, const char *requestedMimeType, Int32 fd)
{
    L_UNUSED(client);

    RDataOffer *rDataOffer { static_cast<RDataOffer*>(wl_resource_get_user_data(resource)) };

    /*
    // If used in drag n drop
    if (rDataOffer->dataOffer()->usedFor() == LDataOffer::DND && seat()->dndManager()->source())
    {
        // Ask the source client to write the data to the FD given the mime type
        seat()->dndManager()->source()->dataSourceResource()->send(mime_type, fd);
    }*/

    // If used in clipboard
    if (rDataOffer->usage() == RDataSource::Clipboard)
    {
        // Louvre keeps a copy of the source clipboard for each mime type (so we don't ask the source client to write the data)
        for (const auto &mimeType : seat()->clipboard()->mimeTypes())
        {
            if (mimeType.mimeType == requestedMimeType)
            {
                if (seat()->clipboard()->m_dataSource.get())
                {
                    seat()->clipboard()->m_dataSource.get()->send(requestedMimeType, fd);
                }
                else if (mimeType.tmp)
                {
                    fseek(mimeType.tmp, 0L, SEEK_END);

                    Int64 total { ftell(mimeType.tmp) };

                    // If pointer is at the beggining means the source client has not written any data
                    if (total == 0)
                        break;

                    rewind(mimeType.tmp);

                    Int64 written = 0, readN = 0, toRead = 0, offset = 0;
                    UChar8 buffer[1024];

                    while (total > 0)
                    {
                        if (total < 1024)
                            toRead = total;
                        else
                            toRead = 1024;

                        readN = fread(buffer, sizeof(UChar8), toRead, mimeType.tmp);

                        if (readN != toRead)
                            break;

                        offset = 0;

                        retryWrite:
                        written = write(fd, &buffer[offset], readN);

                        if (written > 0)
                            offset += written;
                        else if (written == -1)
                            break;
                        else if (written == 0)
                            goto retryWrite;

                        if (offset != readN)
                            goto retryWrite;

                        total -= readN;
                    }
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
    /* TODO
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

    if (!seat()->dndManager()->focus())
        return;

    if (seat()->dndManager()->focus()->client() != rDataOffer->client())
        return;

    rDataOffer->dataOffer()->imp()->acceptedActions = dnd_actions;
    rDataOffer->dataOffer()->imp()->preferredAction = preferred_action;
    rDataOffer->dataOffer()->imp()->updateDNDAction();
    */
}
#endif
