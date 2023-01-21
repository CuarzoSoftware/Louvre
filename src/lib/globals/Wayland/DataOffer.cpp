#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <globals/Wayland/DataOffer.h>

#include <LClient.h>
#include <LCompositor.h>
#include <LSeat.h>

#include <cstring>
#include <stdio.h>
#include <unistd.h>

using namespace Louvre::Globals;

void DataOffer::resource_destroy(wl_resource *resource)
{
    LDataOffer *lDataOffer = (LDataOffer*)wl_resource_get_user_data(resource);

    if(lDataOffer->seat()->dndManager()->imp()->offer == lDataOffer)
    {
        lDataOffer->seat()->dndManager()->imp()->offer = nullptr;
    }

    delete lDataOffer;
}

void DataOffer::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void DataOffer::accept(wl_client *, wl_resource *resource, UInt32 /*serial*/, const char *mime_type)
{
    LDataOffer *lDataOffer = (LDataOffer*)wl_resource_get_user_data(resource);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if(wl_resource_get_version(resource) >= 3 && lDataOffer->imp()->hasFinished)
        wl_resource_post_error(resource,WL_DATA_OFFER_ERROR_INVALID_FINISH,"Invalid DND action mask.");
#endif

    if(mime_type != NULL)
        lDataOffer->seat()->dndManager()->imp()->matchedMimeType = true;

}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void DataOffer::finish(wl_client *, wl_resource *resource)
{
    LDataOffer *lDataOffer = (LDataOffer*)wl_resource_get_user_data(resource);

    if(lDataOffer->usedFor() != LDataOffer::DND)
    {
        wl_resource_post_error(resource,WL_DATA_OFFER_ERROR_INVALID_FINISH,"Data offer not used for DND.");
        return;
    }

    lDataOffer->imp()->hasFinished = true;

    if(lDataOffer->seat()->dndManager()->source() && wl_resource_get_version(lDataOffer->seat()->dndManager()->source()->resource()) >= 3)
        wl_data_source_send_dnd_finished(lDataOffer->seat()->dndManager()->source()->resource());

    if(lDataOffer->seat()->dndManager()->focus() && lDataOffer->seat()->dndManager()->focus()->client()->dataDevice())
        lDataOffer->seat()->dndManager()->focus()->client()->dataDevice()->imp()->sendDNDLeaveEvent();

    lDataOffer->seat()->dndManager()->imp()->clear();

}
#endif

void DataOffer::receive(wl_client *, wl_resource *resource, const char *mime_type, Int32 fd)
{
    LDataOffer *lDataOffer = (LDataOffer*)wl_resource_get_user_data(resource);


    // If used in drag n drop
    if(lDataOffer->usedFor() == LDataOffer::DND && lDataOffer->dataDevice()->seat()->dndManager()->source())
    {
        wl_data_source_send_send(lDataOffer->dataDevice()->seat()->dndManager()->source()->resource(),
                                 mime_type,
                                 fd);
    }

    // If used in clipboard
    else if(lDataOffer->usedFor() == LDataOffer::Selection && lDataOffer->dataDevice()->seat()->dataSelection())
    {

        for(LDataSource::LSource &s : lDataOffer->dataDevice()->seat()->dataSelection()->imp()->sources)
        {
            //printf("Comp %s %s\n",s.mimeType,mime_type);

            if(strcmp(s.mimeType,mime_type) == 0)
            {
                fseek(s.tmp, 0L, SEEK_END);

                // If pointer is at the beggining means the source client has not written any data
                if(ftell(s.tmp) == 0)
                    break;

                rewind(s.tmp);
                char byte = fgetc(s.tmp);
                while(!feof(s.tmp))
                {
                    write(fd,&byte,1);
                    //printf("Wrote %c\n",byte);
                    byte = fgetc(s.tmp);
                }


                break;
            }
        }
    }

    close(fd);
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

void DataOffer::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action)
{
    LDataOffer *lDataOffer = (LDataOffer*)wl_resource_get_user_data(resource);

    if(lDataOffer->imp()->acceptedActions == dnd_actions && lDataOffer->imp()->preferredAction == preferred_action)
        return;

    lDataOffer->imp()->acceptedActions = dnd_actions;
    lDataOffer->imp()->preferredAction = preferred_action;
    lDataOffer->imp()->updateDNDAction();
}
#endif
