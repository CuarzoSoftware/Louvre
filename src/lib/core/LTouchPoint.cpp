#include <private/LTouchPointPrivate.h>
#include <protocols/Wayland/private/RTouchPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LTouchPrivate.h>
#include <LCompositor.h>
#include <LTouchMoveEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <LTouchUpEvent.h>
#include <LClient.h>
#include <LSeat.h>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

LTouchPoint::LTouchPoint(const LTouchDownEvent &event) :
    LPRIVATE_INIT_UNIQUE(LTouchPoint)
{
    imp()->lastDownEvent = event;
    seat()->touch()->imp()->touchPoints.push_back(this);
    imp()->link = std::prev(seat()->touch()->imp()->touchPoints.end());
}

LTouchPoint::~LTouchPoint() {}

Int32 LTouchPoint::id() const
{
    return imp()->lastDownEvent.id();
}

bool LTouchPoint::isPressed() const
{
    return imp()->isPressed;
}

LSurface *LTouchPoint::surface() const
{
    return imp()->surface;
}

const LTouchDownEvent &LTouchPoint::lastDownEvent() const
{
    return imp()->lastDownEvent;
}

const LTouchMoveEvent &LTouchPoint::lastMoveEvent() const
{
    return imp()->lastMoveEvent;
}

const LTouchUpEvent &LTouchPoint::lastUpEvent() const
{
    return imp()->lastUpEvent;
}

const LPointF &LTouchPoint::pos() const
{
    return imp()->pos;
}

bool LTouchPoint::sendDownEvent(const LTouchDownEvent &event, LSurface *surface)
{
    if (event.id() != id())
        return false;

    imp()->isPressed = true;
    imp()->lastDownEvent = event;
    imp()->pos = event.pos();

    // Detach prev sourface
    if (imp()->surface && (!surface || imp()->surface != surface))
    {
        for (GSeat *s : imp()->surface->client()->seatGlobals())
        {
            for (RTouch *t : s->touchResources())
            {
                t->up(LTouchUpEvent(id(), LTime::nextSerial(), event.ms(), event.us(), event.device()));
                t->frame();
            }
        }

        imp()->resetSerials();
    }

    imp()->surface = surface;

    if (imp()->surface)
        imp()->sendTouchDownEvent(event);

    return true;
}

bool LTouchPoint::sendMoveEvent(const LTouchMoveEvent &event)
{
    if (event.id() != id())
        return false;

    imp()->lastMoveEvent = event;
    imp()->pos = event.pos();

    if (!surface())
        return true;

    Float24 x = wl_fixed_from_double(event.localPos.x());
    Float24 y = wl_fixed_from_double(event.localPos.y());

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->motion(event.ms(), id(), x, y);

    return true;
}

bool LTouchPoint::sendUpEvent(const LTouchUpEvent &event)
{
    if (event.id() != id())
        return false;

    imp()->lastUpEvent = event;
    imp()->isPressed = false;

    if (!surface())
        return true;

    for (GSeat *s : surface()->client()->seatGlobals())
        for (RTouch *t : s->touchResources())
            t->up(event);

    return true;
}
