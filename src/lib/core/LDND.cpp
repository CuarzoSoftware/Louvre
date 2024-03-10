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
    if (!m_session.get())
        return;

    if (!surface)
    {
        sendLeaveEvent();
        return;
    }
    else
    {
        // If the source is NULL, only surfaces from the src client are allowed to gain focus
        if (!m_session->source.get() && surface->client() != origin()->client())
        {
            sendLeaveEvent();
            return;
        }
    }

    if (surface == focus())
        return;
    else
    {
        sendLeaveEvent();

        for (auto *gSeat : surface->client()->seatGlobals())
        {
            if (gSeat->dataDeviceResource())
            {
                m_session->focus.reset(surface);
                m_session->dstDataDevice.reset(gSeat->dataDeviceResource());
                break;
            }
        }
    }

    // If the client has zero data devices
    if (!focus())
        return;

    const Float24 x { wl_fixed_from_double(localPos.x()) };
    const Float24 y { wl_fixed_from_double(localPos.y()) };
    const UInt32 serial { LTime::nextSerial() };

    if (m_session->source.get())
    {
        m_session->offer.reset(m_session->dstDataDevice.get()->createOffer(RDataSource::DND));
        m_session->offer.get()->imp()->dndSession = m_session;

        m_session->dstDataDevice.get()->imp()->serials.enter = serial;
        m_session->dstDataDevice.get()->enter(
            serial,
            surface->surfaceResource(),
            x,
            y,
            m_session->offer.get());

        m_session->offer.get()->sourceActions(m_session->source.get()->actions());
    }
    /* If source is NULL, enter, leave and motion events are sent only to the client that
     * initiated the drag and the client is expected to handle the data passing internally */
    else
    {
        m_session->dstDataDevice.get()->imp()->serials.enter = serial;
        m_session->dstDataDevice.get()->enter(
            serial,
            focus()->surfaceResource(),
            x,
            y,
            nullptr);
    }
}

void LDND::sendMoveEvent(const LPointF &localPos, UInt32 ms) noexcept
{
    if (!focus() || !m_session.get() || !m_session.get()->dstDataDevice.get())
        return;

    m_session.get()->dstDataDevice.get()->motion(
        ms,
        wl_fixed_from_double(localPos.x()),
        wl_fixed_from_double(localPos.y()));
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
    if (!m_session.get())
        return;

    if (m_session->source.get())
    {
        m_session->source.get()->cancelled();
        m_session->source.get()->dndFinished();
    }

    sendLeaveEvent();
    cancelled();
    m_session.reset();
}

void LDND::drop() noexcept
{
    if (!dragging())
        return;

    m_session->dropped = true;

    if (icon() && icon()->surface())
        icon()->surface()->imp()->setMapped(false);

    if (!focus())
    {
        cancel();
        return;
    }

    const bool cancelled { m_session->offer.get() && m_session->offer.get()->version() >= 3 && !m_session->offer.get()->matchedMimeType() };

    if (m_session->dstDataDevice.get())
    {
        if (!cancelled)
            m_session->dstDataDevice.get()->drop();

        m_session->dstDataDevice.get()->leave();
    }

    if (m_session->source.get())
    {
        m_session->source.get()->dndDropPerformed();

        if (cancelled)
            m_session->source.get()->cancelled();
    }

    m_session.reset();
}

// Since 3

LDND::Action LDND::preferredAction() const noexcept
{
    return static_cast<Action>(m_compositorAction);
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

void LDND::sendLeaveEvent() noexcept
{
    if (!m_session.get())
        return;

    m_session->focus.reset();

    if (m_session->dstDataDevice.get())
    {
        m_session->dstDataDevice.get()->leave();
        m_session->dstDataDevice.reset();
    }

    if (m_session->offer.get())
    {
        m_session->offer.get()->imp()->dndSession.reset();
        m_session->offer.reset();
    }
}
