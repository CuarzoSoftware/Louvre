#include <protocols/Wayland/private/RTouchPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <LCompositor.h>
#include <LTouchMoveEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <LTouchUpEvent.h>
#include <LTouch.h>
#include <LClient.h>
#include <LSeat.h>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

LTouchPoint::LTouchPoint(const LTouchDownEvent &event) noexcept : m_lastDownEvent(event)
{
    seat()->touch()->m_touchPoints.push_back(this);

    m_surface.setOnDestroyCallback([this](auto)
    {
        resetSerials();
    });
}

void LTouchPoint::sendTouchDownEvent(const LTouchDownEvent &event) noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->down(event, surface()->surfaceResource());
}

void LTouchPoint::sendTouchFrameEvent() noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->frame();
}

void LTouchPoint::sendTouchCancelEvent() noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->cancel();
}

void LTouchPoint::resetSerials() noexcept
{
    m_lastDownEvent.setSerial(0);
    m_lastUpEvent.setSerial(0);
    m_lastMoveEvent.setSerial(0);
}

bool LTouchPoint::sendDownEvent(const LTouchDownEvent &event, LSurface *surf) noexcept
{
    if (event.id() != id())
        return false;

    m_pressed = true;
    m_lastDownEvent = event;
    m_pos = event.pos();

    if (surface() && (!surf || surface() != surf))
    {
        for (GSeat *s : surface()->client()->seatGlobals())
        {
            for (RTouch *t : s->touchResources())
            {
                t->up(LTouchUpEvent(id(), LTime::nextSerial(), event.ms(), event.us(), event.device()));
                t->frame();
            }
        }

        resetSerials();
    }

    m_surface.reset(surf);
    sendTouchDownEvent(event);
    return true;
}

bool LTouchPoint::sendMoveEvent(const LTouchMoveEvent &event) noexcept
{
    if (event.id() != id())
        return false;

    m_lastMoveEvent = event;
    m_pos = event.pos();

    if (!surface())
        return true;

    const Float24 x { wl_fixed_from_double(event.localPos.x()) };
    const Float24 y { wl_fixed_from_double(event.localPos.y()) };

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->motion(event.ms(), id(), x, y);

    return true;
}

bool LTouchPoint::sendUpEvent(const LTouchUpEvent &event) noexcept
{
    if (event.id() != id())
        return false;

    m_lastUpEvent = event;
    m_pressed = false;

    if (!surface())
        return true;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->up(event);

    return true;
}
