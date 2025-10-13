#include <CZ/Louvre/Protocols/Wayland/RTouch.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Core/Events/CZTouchMoveEvent.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Core/Events/CZTouchUpEvent.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Seat/LSeat.h>

using namespace CZ;
using namespace CZ::Protocols::Wayland;

LTouchPoint::LTouchPoint(const CZTouchDownEvent &event) noexcept : m_lastDownEvent(event)
{
    seat()->touch()->m_touchPoints.push_back(this);

    m_surface.setOnDestroyCallback([this](auto)
    {
        resetSerials();
    });
}

void LTouchPoint::sendTouchDownEvent(const CZTouchDownEvent &event) noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchRes())
            t->down(event, surface()->surfaceResource());
}

void LTouchPoint::sendTouchFrameEvent() noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchRes())
            t->frame();
}

void LTouchPoint::sendTouchCancelEvent() noexcept
{
    if (!surface())
        return;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchRes())
            t->cancel();
}

void LTouchPoint::resetSerials() noexcept
{
    m_lastDownEvent.serial = 0;
    m_lastUpEvent.serial = 0;
    m_lastMoveEvent.serial = 0;
}

bool LTouchPoint::sendDownEvent(const CZTouchDownEvent &event, LSurface *surf) noexcept
{
    if (event.id != id())
        return false;

    m_pressed = true;
    m_lastDownEvent = event;
    m_pos = event.pos;

    if (surface() && (!surf || surface() != surf))
    {
        auto touchUp { CZTouchUpEvent() };
        touchUp.id = id();
        touchUp.ms = event.ms;
        touchUp.us = event.us;
        touchUp.device = event.device;

        for (GSeat *s : surface()->client()->seatGlobals())
        {
            for (RTouch *t : s->touchRes())
            {
                touchUp.serial = CZTime::NextSerial();
                t->up(touchUp);
                t->frame();
            }
        }

        resetSerials();
    }

    m_surface.reset(surf);
    sendTouchDownEvent(event);
    return true;
}

bool LTouchPoint::sendMoveEvent(const CZTouchMoveEvent &event) noexcept
{
    if (event.id != id())
        return false;

    m_lastMoveEvent = event;
    m_pos = event.pos;

    if (!surface())
        return true;

    const Float24 x { wl_fixed_from_double(event.localPos.x()) };
    const Float24 y { wl_fixed_from_double(event.localPos.y()) };

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchRes())
            t->motion(event.ms, id(), x, y);

    return true;
}

bool LTouchPoint::sendUpEvent(const CZTouchUpEvent &event) noexcept
{
    if (event.id != id())
        return false;

    m_lastUpEvent = event;
    m_pressed = false;

    if (!surface())
        return true;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchRes())
            t->up(event);

    return true;
}
