#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataOffer.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LDNDIconRole.h>
#include <LDNDSession.h>
#include <LClient.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LDND.h>
#include <LTimer.h>
#include <cassert>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

LDND::LDND(const void *params) noexcept : LFactoryObject(FactoryObjectType)
{
    assert(params != nullptr && "Invalid parameter passed to LDND constructor.");
    LDND **ptr { (LDND**) params };
    assert(*ptr == nullptr && *ptr == seat()->dnd() && "Only a single LDND instance can exist.");
    *ptr = this;
}

void LDND::setFocus(LSurface *surface, const LPointF &localPos) noexcept
{
    if (!m_session)
        return;

    if (!surface)
    {
        sendLeaveEvent();
        return;
    }
    else
    {
        // If the source is NULL, only surfaces from the src client are allowed to gain focus
        if (!m_session->source && surface->client() != origin()->client())
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
            if (gSeat->dataDeviceRes())
            {
                m_session->focus.reset(surface);
                m_session->dstDataDevice.reset(gSeat->dataDeviceRes());
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

    if (m_session->source)
    {
        m_session->offer.reset(m_session->dstDataDevice->createOffer(RDataSource::DND));
        m_session->offer->m_dndSession = m_session;

        m_session->dstDataDevice->m_enterSerial = serial;
        m_session->dstDataDevice->enter(
            serial,
            surface->surfaceResource(),
            x,
            y,
            m_session->offer);

        m_session->offer->sourceActions(m_session->source->actions());
    }
    /* If source is NULL, enter, leave and motion events are sent only to the client that
     * initiated the drag and the client is expected to handle the data passing internally */
    else
    {
        m_session->dstDataDevice->m_enterSerial = serial;
        m_session->dstDataDevice->enter(
            serial,
            focus()->surfaceResource(),
            x,
            y,
            nullptr);
    }
}

void LDND::sendMoveEvent(const LPointF &localPos, UInt32 ms) noexcept
{
    if (!focus() || !m_session || !m_session->dstDataDevice)
        return;

    m_session->dstDataDevice->motion(
        ms,
        wl_fixed_from_double(localPos.x()),
        wl_fixed_from_double(localPos.y()));
}

const LEvent &LDND::triggeringEvent() const noexcept
{
    return *m_triggeringEvent;
}

LDNDIconRole *LDND::icon() const noexcept
{
    if (m_session)
        return m_session->icon;
    return nullptr;
}

LSurface *LDND::origin() const noexcept
{
    if (m_session)
        return m_session->origin;
    return nullptr;
}

LSurface *LDND::focus() const noexcept
{
    if (m_session)
        return m_session->focus;
    return nullptr;
}

bool LDND::dragging() const noexcept
{
    return m_session != nullptr;
}

void LDND::cancel() noexcept
{
    if (!m_session)
        return;

    if (m_session->source)
    {
        m_session->source->cancelled();
        m_session->source->dndFinished();
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

    const bool cancelled { m_session->offer && m_session->offer->version() >= 3 && !m_session->offer->matchedMimeType() };

    if (m_session->dstDataDevice)
    {
        if (!cancelled)
            m_session->dstDataDevice->drop();

        m_session->dstDataDevice->leave();
    }

    if (m_session->source)
    {
        m_session->source->dndDropPerformed();

        if (cancelled)
            m_session->source->cancelled();
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
    if (!m_session)
        return;

    m_session->focus.reset();

    if (m_session->dstDataDevice)
    {
        m_session->dstDataDevice->leave();
        m_session->dstDataDevice.reset();
    }

    if (m_session->offer)
    {
        m_session->offer->m_dndSession.reset();
        m_session->offer.reset();
    }
}
