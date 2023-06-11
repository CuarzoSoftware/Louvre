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

LDataDevice::LDataDevice()
{
    m_imp = new LDataDevicePrivate();
}

LDataDevice::~LDataDevice()
{
    delete m_imp;
}

LClient *LDataDevice::client() const
{
    return imp()->client;
}

void LDataDevice::sendSelectionEvent()
{
    // Send data device selection first
    if (seat()->dataSelection())
    {
        for (Wayland::GSeat *d : client()->seatGlobals())
        {
            if (d->dataDeviceResource())
            {
                Wayland::RDataOffer *rDataOffer = new Wayland::RDataOffer(d->dataDeviceResource(), 0);
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
    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus() != surface)
    {
        sendDNDLeaveEvent();

        UInt32 serial = LCompositor::nextSerial();

        if (seat()->dndManager()->source())
        {
            for (Wayland::GSeat *d : client->seatGlobals())
            {
                if (d->dataDeviceResource())
                {
                    Wayland::RDataOffer *rDataOffer = new Wayland::RDataOffer(d->dataDeviceResource(), 0);

                    rDataOffer->dataOffer()->imp()->usedFor = LDataOffer::DND;
                    d->dataDeviceResource()->imp()->dataOffered = rDataOffer->dataOffer();
                    d->dataDeviceResource()->dataOffer(rDataOffer);

                    for (const LDataSource::LSource &s : seat()->dndManager()->source()->sources())
                        rDataOffer->offer(s.mimeType);

                    d->dataDeviceResource()->imp()->serials.enter = serial;
                    d->dataDeviceResource()->enter(serial,
                                                   surface->surfaceResource(),
                                                   x,
                                                   y,
                                                   rDataOffer);

                    rDataOffer->sourceActions(seat()->dndManager()->source()->dndActions());
                }
            }

            seat()->dndManager()->imp()->focus = surface;
        }
        else
        {
            if (surface == seat()->dndManager()->origin())
            {
                for (Wayland::GSeat *d : client->seatGlobals())
                {
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
}

void LDataDevice::LDataDevicePrivate::sendDNDMotionEventS(Float24 x, Float24 y)
{
    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus())
        if (seat()->dndManager()->source() || (!seat()->dndManager()->source() && seat()->dndManager()->focus() == seat()->dndManager()->origin()))
        {
            UInt32 ms = LTime::ms();
            for (Wayland::GSeat *d : client->seatGlobals())
                if (d->dataDeviceResource())
                    d->dataDeviceResource()->motion(ms, x, y);
        }
}

void LDataDevice::LDataDevicePrivate::sendDNDLeaveEvent()
{
    if (seat()->dndManager()->dragging() && seat()->dndManager()->focus())
        for (Wayland::GSeat *d : client->seatGlobals())
            if (d->dataDeviceResource())
                d->dataDeviceResource()->leave();

    seat()->dndManager()->imp()->matchedMimeType = false;
    seat()->dndManager()->imp()->focus = nullptr;
}
