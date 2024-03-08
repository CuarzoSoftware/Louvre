#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LDNDIconRole.h>
#include <LDNDSession.h>
#include <LClient.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LDND.h>
#include <LTimer.h>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

void LDND::setFocus(LSurface *surface, const LPointF &localPos) noexcept
{
    /* TODO
    if (!m_session)
        return;

    if (!surface)
    {
        sendLeaveEvent(focus());
        return;
    }
    else
    {
        // If the source is NULL, only surfaces from the src client are allowed to gain focus
        if (!source() && surface->client() != origin()->client())
        {
            sendLeaveEvent(focus());
            return;
        }
    }

    if (surface == focus())
        return;
    else
    {
        sendLeaveEvent(focus());
        focus = surface;
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
*/
}

void LDND::sendMoveEvent(const LPointF &localPos, UInt32 ms) noexcept
{
    /* TODO
    if (!focus() || seat()->dndManager()->imp()->dropped)
        return;

    const Float24 x { wl_fixed_from_double(localPos.x()) };
    const Float24 y { wl_fixed_from_double(localPos.y()) };

    for (GSeat *gSeat : focus()->client()->seatGlobals())
        if (gSeat->dataDeviceResource())
            gSeat->dataDeviceResource()->motion(ms, x, y);
    */
}

const LEvent &LDND::triggeringEvent() const noexcept
{
    return *m_triggeringEvent.get();
}

LDNDIconRole *LDND::icon() const noexcept
{
    if (m_session)
        return m_session->icon.get();
    return nullptr;
}

LSurface *LDND::origin() const noexcept
{
    if (m_session)
        return m_session->origin.get();
    return nullptr;
}

LSurface *LDND::focus() const noexcept
{
    if (m_session)
        return m_session->focus.get();
    return nullptr;
}

bool LDND::dragging() const noexcept
{
    return m_session.get() != nullptr;
}

void LDND::cancel() noexcept
{
    if (m_session)
        return;

    if (m_session->source.get())
    {
        m_session->source.get()->cancelled();
        m_session->source.get()->dndFinished();
    }

    sendLeaveEvent(focus());
    cancelled();
    m_session.reset();
}

void LDND::drop() noexcept
{
    /* TODO
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
*/
}

// Since 3

LDND::Action LDND::preferredAction() const noexcept
{
    return (Action)m_compositorAction;
}

void LDND::setPreferredAction(LDND::Action action) noexcept
{
    if (m_compositorAction == action)
        return;

    m_compositorAction = action;

    if (m_session)
    {
        m_session->compositorAction = action;
        m_session->updateActions();
    }
}

void LDND::sendLeaveEvent(LSurface *surface) noexcept
{
    /* TODO
    matchedMimeType = false;
    focus = nullptr;

    if (!surface)
        return;

    for (auto seatGlobal : surface->client()->seatGlobals())
        if (seatGlobal->dataDeviceResource())
            seatGlobal->dataDeviceResource()->leave();
    */
}
