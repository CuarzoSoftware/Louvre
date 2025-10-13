#include <CZ/Louvre/Protocols/Wayland/GCompositor.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_compositor_interface imp
{
    .create_surface = &GCompositor::create_surface,
    .create_region = &GCompositor::create_region
};

LGLOBAL_INTERFACE_IMP(GCompositor, LOUVRE_WL_COMPOSITOR_VERSION, wl_compositor_interface)

bool GCompositor::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.WlCompositor)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlCompositor;
    return true;
}

GCompositor::GCompositor
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
    this->client()->imp()->compositorGlobals.push_back(this);
}

GCompositor::~GCompositor() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->compositorGlobals, this);
}

/******************** REQUESTS ********************/

void GCompositor::create_surface(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    new RWlSurface(static_cast<GCompositor*>(wl_resource_get_user_data(resource)), id);
}

void GCompositor::create_region(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RRegion(static_cast<GCompositor*>(wl_resource_get_user_data(resource)), id);
}
