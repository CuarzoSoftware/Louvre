#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <LClipboard.h>
#include <LDNDSession.h>
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
    L_UNUSED(client);
    L_UNUSED(serial);

    RDataOffer *rDataOffer { static_cast<RDataOffer*>(wl_resource_get_user_data(resource)) };

    rDataOffer->imp()->matchedMimeType = mime_type != NULL;

    if (rDataOffer->imp()->dndSession.get() && rDataOffer->imp()->dndSession.get()->source.get())
        rDataOffer->imp()->dndSession.get()->source.get()->target(mime_type);
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::RDataOfferPrivate::finish(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    RDataOffer *rDataOffer { static_cast<RDataOffer*>(wl_resource_get_user_data(resource)) };

    if (rDataOffer->usage() != RDataSource::DND)
    {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH, "Data offer not used for DND.");
        return;
    }

    if (rDataOffer->imp()->dndSession.get() && rDataOffer->imp()->dndSession.get()->source.get())
        rDataOffer->imp()->dndSession.get()->source.get()->dndFinished();

    rDataOffer->imp()->dndSession.reset();
}
#endif

void RDataOffer::RDataOfferPrivate::receive(wl_client *client, wl_resource *resource, const char *requestedMimeType, Int32 fd)
{
    L_UNUSED(client);

    RDataOffer *rDataOffer { static_cast<RDataOffer*>(wl_resource_get_user_data(resource)) };

    if (rDataOffer->usage() == RDataSource::DND)
    {
        if (rDataOffer->imp()->dndSession.get() && rDataOffer->imp()->dndSession.get()->source.get())
            rDataOffer->imp()->dndSession.get()->source.get()->send(requestedMimeType, fd);
    }
    else if (rDataOffer->usage() == RDataSource::Clipboard)
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
    L_UNUSED(client);

    RDataOffer *rDataOffer { static_cast<RDataOffer*>(wl_resource_get_user_data(resource)) };

    if (rDataOffer->usage() != RDataSource::DND)
    {
        wl_resource_post_error(resource, -1, "Data offer not being used for DND.");
        return;
    }

    if (rDataOffer->actions() == dnd_actions && rDataOffer->preferredAction() == preferred_action)
        return;

    dnd_actions &= LDND::Copy | LDND::Move | LDND::Ask;

    if (preferred_action != LDND::NoAction && preferred_action != LDND::Copy && preferred_action != LDND::Move && preferred_action != LDND::Ask)
    {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION, "Invalid preferred_action.");
        return;
    }

    rDataOffer->imp()->actions = dnd_actions;
    rDataOffer->imp()->preferredAction = preferred_action;

    if (rDataOffer->imp()->dndSession.get())
        rDataOffer->imp()->dndSession.get()->updateActions();
}
#endif
