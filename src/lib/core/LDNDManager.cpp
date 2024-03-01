#include "LLog.h"
#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LDNDIconRole.h>
#include <LClient.h>
#include <LDataSource.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LTimer.h>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

void LDNDManager::setFocus(LSurface *surface, const LPointF &localPos)
{
    if (!surface)
    {
        imp()->sendLeaveEvent(focus());
        return;
    }

    if (surface == focus() || seat()->dndManager()->imp()->dropped)
        return;
    else
    {
        imp()->sendLeaveEvent(focus());
        imp()->focus = surface;
    }

    const Float24 x { wl_fixed_from_double(localPos.x()) };
    const Float24 y { wl_fixed_from_double(localPos.y()) };

    if (source())
    {
        const UInt32 serial { LTime::nextSerial() };

        for (GSeat *gSeat : focus()->client()->seatGlobals())
        {
            if (gSeat->dataDeviceResource())
            {
                RDataOffer *rDataOffer = new RDataOffer(gSeat->dataDeviceResource(), 0);

                rDataOffer->dataOffer()->imp()->usedFor = LDataOffer::DND;
                gSeat->dataDeviceResource()->imp()->dataOffered = rDataOffer->dataOffer();
                gSeat->dataDeviceResource()->dataOffer(rDataOffer);

                for (const LDataSource::LSource &s : source()->sources())
                    rDataOffer->offer(s.mimeType);

                gSeat->dataDeviceResource()->imp()->serials.enter = serial;
                gSeat->dataDeviceResource()->enter(serial,
                                                   surface->surfaceResource(),
                                                   x,
                                                   y,
                                                   rDataOffer);

                rDataOffer->sourceActions(source()->dndActions());
            }
        }
    }
    // If source is NULL, enter, leave and motion events are sent only to the client that
    // initiated the drag and the client is expected to handle the data passing internally
    else if (origin() == focus())
    {
        const UInt32 serial { LTime::nextSerial() };

        for (GSeat *gSeat : focus()->client()->seatGlobals())
        {
            if (gSeat->dataDeviceResource())
            {
                gSeat->dataDeviceResource()->imp()->serials.enter = serial;
                gSeat->dataDeviceResource()->enter(
                    serial,
                    surface->surfaceResource(),
                    x,
                    y,
                    NULL);
            }
        }
    }
}

void LDNDManager::sendMoveEvent(const LPointF &localPos, UInt32 ms)
{
    if (!focus() || seat()->dndManager()->imp()->dropped)
        return;

    const Float24 x { wl_fixed_from_double(localPos.x()) };
    const Float24 y { wl_fixed_from_double(localPos.y()) };

    for (GSeat *gSeat : focus()->client()->seatGlobals())
        if (gSeat->dataDeviceResource())
            gSeat->dataDeviceResource()->motion(ms, x, y);
}

const LEvent &LDNDManager::triggeringEvent() const
{
    return *imp()->triggeringEvent.get();
}

LDNDManager::LDNDManager(const void *params) : LPRIVATE_INIT_UNIQUE(LDNDManager)
{
    L_UNUSED(params);
}

LDNDManager::~LDNDManager() {}

LDNDIconRole *LDNDManager::icon() const
{
    return imp()->icon;
}

LSurface *LDNDManager::origin() const
{
    return imp()->origin;
}

LSurface *LDNDManager::focus() const
{
    return imp()->focus;
}

LDataSource *LDNDManager::source() const
{
    return imp()->source;
}

Wayland::RDataDevice *LDNDManager::srcDataDevice() const
{
    return imp()->srcDataDevice;
}

LClient *LDNDManager::dstClient() const
{
    return imp()->dstClient;
}

bool LDNDManager::dragging() const
{
    return imp()->origin != nullptr && !imp()->dropped;
}

void LDNDManager::cancel()
{
    if (source())
    {
        source()->dataSourceResource()->cancelled();
        source()->dataSourceResource()->dndFinished();
    }

    imp()->sendLeaveEvent(focus());
    imp()->clear();
    cancelled();
}

void LDNDManager::drop()
{
    if (!dragging())
        return;

    if (!focus())
    {
        cancel();
        return;
    }

    if (!imp()->dropped)
    {
        imp()->dropped = true;

        LTimer::oneShot(100, [this](LTimer *)
        {
            if (source() && imp()->dropped)
                cancel();
        });

        compositor()->imp()->unlockPoll();

        if (icon() && icon()->surface())
            icon()->surface()->imp()->setMapped(false);

        if (imp()->focus)
        {
            for (Wayland::GSeat *s : imp()->focus->client()->seatGlobals())
            {
                if (s->dataDeviceResource())
                {
                    if (!imp()->matchedMimeType && s->dataDeviceResource()->version() >= 3)
                    {
                        cancel();
                        return;
                    }

                    s->dataDeviceResource()->drop();
                }
            }

            if (source())
                source()->dataSourceResource()->dndDropPerformed();
        }
        else
        {
            if (source())
                source()->dataSourceResource()->dndDropPerformed();

            cancel();
        }
    }
}

// Since 3

LDNDManager::Action LDNDManager::preferredAction() const
{
    return imp()->preferredAction;
}

void LDNDManager::setPreferredAction(LDNDManager::Action action)
{
    if (imp()->preferredAction == action)
        return;

    imp()->preferredAction = action;

    if (focus())
        for (Wayland::GSeat *s : focus()->client()->seatGlobals())
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
}
