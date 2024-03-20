#include <protocols/Wayland/RTouch.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LClientPrivate.h>
#include <algorithm>

using namespace Louvre::Protocols::Wayland;

static const struct wl_touch_interface imp
{
#if LOUVRE_WL_SEAT_VERSION >= 3
    .release = &RTouch::release
#endif
};

RTouch::RTouch
    (
        GSeat *seatRes,
        Int32 id
    ) noexcept
    :LResource
    (
        seatRes->client(),
        &wl_touch_interface,
        seatRes->version(),
        id,
        &imp
    ),
    m_seatRes(seatRes)
{
    seatRes->m_touchRes.emplace_back(this);
}

RTouch::~RTouch() noexcept
{
    if (seatRes())
        LVectorRemoveOneUnordered(seatRes()->m_touchRes, this);
}

#if LOUVRE_WL_SEAT_VERSION >= 3
void RTouch::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif


void RTouch::down(const LTouchDownEvent &event, RSurface *surfaceRes) noexcept
{
    auto &clientEvents { client()->imp()->events.touch.down };

    auto it = std::find_if(
        clientEvents.begin(),
        clientEvents.end(),
        [event](const LTouchDownEvent &ev)
    {
        return ev.id() == event.id();
    });

    if (it == clientEvents.end())
        clientEvents.push_back(event);
    else if (it->serial() != event.serial())
        *it = event;

    wl_touch_send_down(resource(),
                       event.serial(),
                       event.ms(),
                       surfaceRes->resource(),
                       event.id(),
                       wl_fixed_from_double(event.localPos.x()),
                       wl_fixed_from_double(event.localPos.y()));
}

void RTouch::up(const LTouchUpEvent &event) noexcept
{
    auto &clientEvents { client()->imp()->events.touch.up };

    auto it = std::find_if(
        clientEvents.begin(),
        clientEvents.end(),
        [event](const LTouchUpEvent &ev)
        {
            return ev.id() == event.id();
        });

    if (it == clientEvents.end())
        clientEvents.push_back(event);
    else if (it->serial() != event.serial())
        *it = event;

    wl_touch_send_up(resource(), event.serial(), event.ms(), event.id());
}

void RTouch::motion(UInt32 time, Int32 id, Float24 x, Float24 y) noexcept
{
    wl_touch_send_motion(resource(), time, id, x, y);
}

void RTouch::frame() noexcept
{
    wl_touch_send_frame(resource());
}

void RTouch::cancel() noexcept
{
    wl_touch_send_cancel(resource());
}

bool RTouch::shape(Int32 id, Float24 major, Float24 minor) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 6
    if (version() >= 6)
    {
        wl_touch_send_shape(resource(), id, major, minor);
        return true;
    }
#endif
    L_UNUSED(id);
    L_UNUSED(major);
    L_UNUSED(minor);
    return false;
}

bool RTouch::orientation(Int32 id, Float24 orientation) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 6
    if (version() >= 6)
    {
        wl_touch_send_orientation(resource(), id, orientation);
        return true;
    }
#endif
    L_UNUSED(id);
    L_UNUSED(orientation);
    return false;
}
