#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LCursorRolePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Cursor/LRoleCursorSource.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_pointer_interface imp
{
    .set_cursor = &RPointer::set_cursor,
#if LOUVRE_WL_SEAT_VERSION >= 3
    .release = &RPointer::release
#endif
};

RPointer::RPointer
(
    GSeat *seatRes,
    Int32 id
) noexcept
    :LResource
    (
        seatRes->client(),
        &wl_pointer_interface,
        seatRes->version(),
        id,
        &imp
    ),
    m_seatRes(seatRes)
{
    seatRes->m_pointerRes.emplace_back(this);
}

RPointer::~RPointer() noexcept
{
    if (seatRes())
        CZVectorUtils::RemoveOneUnordered(seatRes()->m_pointerRes, this);
}

/******************** REQUESTS ********************/

void RPointer::set_cursor(wl_client */*client*/, wl_resource *resource, UInt32 serial, wl_resource *wlSurface, Int32 hotspot_x, Int32 hotspot_y)
{
    RPointer &pointerRes { *static_cast<RPointer*>(wl_resource_get_user_data(resource)) };
    LClient &client { *pointerRes.client() };

    if (client.eventHistory().pointer.enter.serial != serial)
    {
        LLog(CZWarning, CZLN, "Set cursor request without valid pointer enter event serial. Ignoring it...");
        return;
    }

    if (wlSurface)
    {
        const RWlSurface *surfaceRes { static_cast<RWlSurface*>(wl_resource_get_user_data(wlSurface)) };
        LSurface &surface { *surfaceRes->surface() };
        LCursorRole *cursorRole { surface.cursorRole() };
        bool hadRole { true };

        if (!cursorRole)
        {
            if (!surface.imp()->canHostRole())
            {
                pointerRes.postError(WL_POINTER_ERROR_ROLE, "Given wl_surface has another role.");
                return;
            }

            LCursorRole::Params cursorRoleParams { &surface };
            cursorRole = LFactory::createObject<LCursorRole>(&cursorRoleParams);
            hadRole = false;
        }

        cursorRole->m_hotspot.fX = hotspot_x;
        cursorRole->m_hotspot.fY = hotspot_y;
        cursorRole->m_hotspotB.set(
            cursorRole->m_hotspot.x() * surface.scale(),
            cursorRole->m_hotspot.y() * surface.scale());

        if (!hadRole)
        {
            surface.imp()->notifyRoleChange();
            surface.imp()->setLayer(LLayerOverlay);
        }

        if (surface.image())
        {
            cursorRole->m_cursor->m_image = surface.image();
            cursorRole->m_cursor->m_visibility = LCursorSource::Visible;
        }
        else
        {
            cursorRole->m_cursor->m_visibility = LRoleCursorSource::Hidden;

        }

        cursorRole->m_cursor->m_hotspot = cursorRole->m_hotspotB;
        cursorRole->m_cursor->m_triggeringEvent = client.eventHistory().pointer.enter.copy();
        client.imp()->cursor = cursorRole->m_cursor;
        seat()->pointer()->setCursorRequest(cursorRole->m_cursor);
        return;
    }
    else
    {
        auto source { LRoleCursorSource::MakeDefault(&client) };
        client.imp()->cursor = source;
        source->m_triggeringEvent = client.eventHistory().pointer.enter.copy();
        source->m_visibility = LRoleCursorSource::Hidden;
        seat()->pointer()->setCursorRequest(source);
    }
}

#if LOUVRE_WL_SEAT_VERSION >= 3
void RPointer::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

/******************** EVENTS ********************/

void RPointer::enter(const CZPointerEnterEvent &event, RWlSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.enter };

    if (clientEvent.serial != event.serial)
        clientEvent = event;

    wl_pointer_send_enter(resource(),
                          event.serial,
                          surfaceRes->resource(),
                          wl_fixed_from_double(event.pos.x()),
                          wl_fixed_from_double(event.pos.y()));
}

void RPointer::leave(const CZPointerLeaveEvent &event, RWlSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.leave };

    if (clientEvent.serial != event.serial)
        clientEvent = event;

    wl_pointer_send_leave(resource(), event.serial, surfaceRes->resource());
}

void RPointer::motion(const CZPointerMoveEvent &event) noexcept
{
    wl_pointer_send_motion(resource(),
                           event.ms,
                           wl_fixed_from_double(event.pos.x()),
                           wl_fixed_from_double(event.pos.y()));
}

void RPointer::button(const CZPointerButtonEvent &event) noexcept
{
    auto &clientEvents { client()->imp()->eventHistory.pointer };

    if (clientEvents.button[clientEvents.buttonIndex].serial != event.serial)
    {
        if (clientEvents.buttonIndex == 4)
            clientEvents.buttonIndex = 0;
        else
            clientEvents.buttonIndex++;

        clientEvents.button[clientEvents.buttonIndex] = event;
    }

    wl_pointer_send_button(resource(), event.serial, event.ms, event.button, event.pressed);
}

void RPointer::axis(UInt32 time, UInt32 axis, Float32 value) noexcept
{
    wl_pointer_send_axis(resource(), time, axis, wl_fixed_from_double(value));
}

bool RPointer::frame() noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_frame(resource());
        return true;
    }
#endif
    return false;
}

bool RPointer::axisSource(UInt32 axisSource) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_source(resource(), axisSource);
        return true;
    }
#endif
    CZ_UNUSED(axisSource);
    return false;
}

bool RPointer::axisStop(UInt32 time, UInt32 axis) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_stop(resource(), time, axis);
        return true;
    }
#endif
    CZ_UNUSED(time);
    CZ_UNUSED(axis);
    return false;
}

bool RPointer::axisDiscrete(UInt32 axis, Int32 discrete) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_discrete(resource(), axis, discrete);
        return true;
    }
#endif
    CZ_UNUSED(axis);
    CZ_UNUSED(discrete);
    return false;
}

bool RPointer::axisValue120(UInt32 axis, Int32 value120) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 8
    if (version() >= 8)
    {
        wl_pointer_send_axis_value120(resource(), axis, value120);
        return true;
    }
#endif
    CZ_UNUSED(axis);
    CZ_UNUSED(value120);
    return false;
}

bool RPointer::axisRelativeDirection(UInt32 axis, UInt32 direction) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 9
    if (version() >= 9)
    {
        wl_pointer_send_axis_relative_direction(resource(), axis, direction);
        return true;
    }
#endif
    CZ_UNUSED(axis);
    CZ_UNUSED(direction);
    return false;
}
