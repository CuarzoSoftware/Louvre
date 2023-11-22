#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LDataDevicePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <LTime.h>

using namespace Louvre;

LDataDevice::LDataDevice() : LPRIVATE_INIT_UNIQUE(LDataDevice) {}
LDataDevice::~LDataDevice() {}

LClient *LDataDevice::client() const
{
    return imp()->client;
}

void LDataDevice::sendSelectionEvent()
{
    // Send data device selection first
    if (seat()->dataSelection())
    {
        for (Protocols::Wayland::GSeat *d : client()->seatGlobals())
        {
            if (d->dataDeviceResource())
            {
                Protocols::Wayland::RDataOffer *rDataOffer = new Protocols::Wayland::RDataOffer(d->dataDeviceResource(), 0);
                rDataOffer->dataOffer()->imp()->usedFor = LDataOffer::Selection;
                d->dataDeviceResource()->dataOffer(rDataOffer);

                for (const LDataSource::LSource &s : seat()->dataSelection()->sources())
                    rDataOffer->offer(s.mimeType);

                d->dataDeviceResource()->selection(rDataOffer);
            }
        }
    }
}

void LDataDevice::LDataDevicePrivate::sendDNDEnterEventS(LSurface *surface, Float24 x, Float24 y)
{
    if (!surface)
        return;

    if (seat()->dndManager()->imp()->dropped)
        return;

    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus() != surface)
    {
        if (seat()->dndManager()->focus())
            seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

        if (seat()->dndManager()->source())
        {
            seat()->dndManager()->imp()->focus = surface;

            for (Protocols::Wayland::GSeat *d : client->seatGlobals())
            {
                if (d->dataDeviceResource())
                {
                    Protocols::Wayland::RDataOffer *rDataOffer = new Protocols::Wayland::RDataOffer(d->dataDeviceResource(), 0);

                    rDataOffer->dataOffer()->imp()->usedFor = LDataOffer::DND;
                    d->dataDeviceResource()->imp()->dataOffered = rDataOffer->dataOffer();
                    d->dataDeviceResource()->dataOffer(rDataOffer);

                    for (const LDataSource::LSource &s : seat()->dndManager()->source()->sources())
                        rDataOffer->offer(s.mimeType);

                    UInt32 serial = LCompositor::nextSerial();

                    d->dataDeviceResource()->imp()->serials.enter = serial;
                    d->dataDeviceResource()->enter(serial,
                                                   surface->surfaceResource(),
                                                   x,
                                                   y,
                                                   rDataOffer);

                    rDataOffer->sourceActions(seat()->dndManager()->source()->dndActions());
                }
            }

            sendDNDMotionEventS(x, y);
        }
        // If source is NULL, enter, leave and motion events are sent only to the client that
        // initiated the drag and the client is expected to handle the data passing internally
        else
        {
            if (surface && surface->client() == client)
            {
                for (Protocols::Wayland::GSeat *d : client->seatGlobals())
                {
                    UInt32 serial = LCompositor::nextSerial();

                    if (d->dataDeviceResource())
                    {
                        d->dataDeviceResource()->imp()->serials.enter = serial;
                        d->dataDeviceResource()->enter(serial,
                                                       surface->surfaceResource(),
                                                       x,
                                                       y,
                                                       NULL);
                    }
                }

                seat()->dndManager()->imp()->focus = surface;
            }
        }
    }

    surface->client()->flush();
}

void LDataDevice::LDataDevicePrivate::sendDNDMotionEventS(Float24 x, Float24 y)
{
    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus() && seat()->dndManager()->focus() == seat()->pointer()->focus())
        if (seat()->dndManager()->source() || (!seat()->dndManager()->source() && seat()->dndManager()->srcDataDevice()->client()  && seat()->dndManager()->srcDataDevice()->client() == client))
        {
            UInt32 ms = LTime::ms();
            for (Protocols::Wayland::GSeat *d : client->seatGlobals())
                if (d->dataDeviceResource())
                    d->dataDeviceResource()->motion(ms, x, y);
        }
}

void LDataDevice::LDataDevicePrivate::sendDNDLeaveEvent()
{
    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus())
        for (Protocols::Wayland::GSeat *d : client->seatGlobals())
            if (d->dataDeviceResource())
                d->dataDeviceResource()->leave();

    seat()->dndManager()->imp()->matchedMimeType = false;
    seat()->dndManager()->imp()->focus = nullptr;
}
