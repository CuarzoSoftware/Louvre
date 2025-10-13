#include <CZ/Louvre/Protocols/CursorShape/cursor-shape-v1.h>
#include <CZ/Louvre/Protocols/CursorShape/GCursorShapeManager.h>
#include <CZ/Louvre/Protocols/CursorShape/RCursorShapeDevice.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::CursorShape;

static const struct wp_cursor_shape_manager_v1_interface imp
{
    .destroy = &GCursorShapeManager::destroy,
    .get_pointer = &GCursorShapeManager::get_pointer,
    .get_tablet_tool_v2 = &GCursorShapeManager::get_tablet_tool_v2
};

LGLOBAL_INTERFACE_IMP(GCursorShapeManager, LOUVRE_CURSOR_SHAPE_MANAGER_VERSION, wp_cursor_shape_manager_v1_interface)

bool GCursorShapeManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.CursorShapeManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.CursorShapeManager;
    return true;
}

GCursorShapeManager::GCursorShapeManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->cursorShapeManagerGlobals.emplace_back(this);
}

GCursorShapeManager::~GCursorShapeManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->cursorShapeManagerGlobals, this);
}

void GCursorShapeManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GCursorShapeManager::get_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource */*pointer*/)
{
    auto *manager { static_cast<GCursorShapeManager*>(wl_resource_get_user_data(resource)) };
    new RCursorShapeDevice(manager, RCursorShapeDevice::Pointer, id);
}

void GCursorShapeManager::get_tablet_tool_v2(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *tabletTool)
{
    /* TODO: Implement tablet protocol */
    CZ_UNUSED(client)
    CZ_UNUSED(resource)
    CZ_UNUSED(id)
    CZ_UNUSED(tabletTool)
}
