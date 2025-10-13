#include <CZ/Louvre/Protocols/Wayland/GSubcompositor.h>
#include <CZ/Louvre/Protocols/Wayland/RSubsurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_subcompositor_interface imp
{
    .destroy = &GSubcompositor::destroy,
    .get_subsurface = &GSubcompositor::get_subsurface
};

LGLOBAL_INTERFACE_IMP(GSubcompositor, LOUVRE_WL_SUBCOMPOSITOR_VERSION, wl_subcompositor_interface)

bool GSubcompositor::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.WlSubcompositor)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlSubcompositor;
    return true;
}

GSubcompositor::GSubcompositor
    (
        wl_client *client,
        Int32 version,
        UInt32 id)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->subcompositorGlobals.push_back(this);
}

GSubcompositor::~GSubcompositor() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->subcompositorGlobals, this);
}

/******************** REQUESTS ********************/

void GSubcompositor::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSubcompositor::get_subsurface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent) noexcept
{
    auto &surfaceRes { *static_cast<RWlSurface*>(wl_resource_get_user_data(surface)) };
    auto &parentRes { *static_cast<RWlSurface*>(wl_resource_get_user_data(parent)) };

    if (!surfaceRes.surface()->imp()->canHostRole())
    {
        parentRes.postError(WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "Given wl_surface already has another role");
        return;
    }

    if (LSurface::LSurfacePrivate::IsSubsurfaceOf(parentRes.surface(), surfaceRes.surface()))
    {
        parentRes.postError(WL_SUBCOMPOSITOR_ERROR_BAD_PARENT, "The parent can not be descendant or equal to the surface");
        return;
    }

    new RSubsurface(static_cast<GSubcompositor*>(wl_resource_get_user_data(resource)),
                    surfaceRes.surface(),
                    parentRes.surface(),
                    id);
}
