#include "LLog.h"
#include <CZ/Louvre/Protocols/CursorShape/cursor-shape-v1.h>
#include <CZ/Louvre/Protocols/CursorShape/GCursorShapeManager.h>
#include <CZ/Louvre/Protocols/CursorShape/RCursorShapeDevice.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Cursor/LShapeCursorSource.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Core/CZCursorShape.h>

using namespace CZ;
using namespace CZ::Protocols::CursorShape;

static const struct wp_cursor_shape_device_v1_interface imp
{
    .destroy = &RCursorShapeDevice::destroy,
    .set_shape = &RCursorShapeDevice::set_shape
};

void RCursorShapeDevice::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RCursorShapeDevice::set_shape(wl_client */*client*/, wl_resource *resource, UInt32 serial, UInt32 shape)
{
    auto &res { *static_cast<RCursorShapeDevice*>(wl_resource_get_user_data(resource)) };
    const auto *event { res.client()->findEventBySerial(serial) };

    if (!event)
    {
        LLog(CZTrace, CZLN, "Request ignored (invalid serial)");
        return;
    }

    if (shape == 0 || (res.version() == 1 && shape > 34) || (res.version() == 2 && shape > 36))
    {
        res.postError(WP_CURSOR_SHAPE_DEVICE_V1_ERROR_INVALID_SHAPE, "Invalid shape {}", shape);
        return;
    }

    auto source { LShapeCursorSource::MakeClient((CZCursorShape)shape, res.client(), event->copy()) };
    res.client()->imp()->cursor = source;
    seat()->pointer()->setCursorRequest(source);
}

RCursorShapeDevice::RCursorShapeDevice
    (
        GCursorShapeManager *manager,
        Type type,
        UInt32 id
    ) noexcept
    :LResource
    (
        manager->client(),
        &wp_cursor_shape_device_v1_interface,
        manager->version(),
        id,
        &imp
    ),
    m_type(type)
{}
