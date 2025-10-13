#include <CZ/Louvre/Protocols/SvgPath/GSvgPathManager.h>
#include <CZ/Louvre/Protocols/SvgPath/RSvgPath.h>
#include <CZ/Louvre/Protocols/SvgPath/lvr-svg-path.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::SvgPath;

static const struct lvr_svg_path_manager_interface imp
{
    .destroy = &GSvgPathManager::destroy,
    .get_svg_path = &GSvgPathManager::get_svg_path
};

LGLOBAL_INTERFACE_IMP(GSvgPathManager, LOUVRE_SVG_PATH_MANAGER_VERSION, lvr_svg_path_manager_interface)

bool GSvgPathManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.LvrSvgPathManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.LvrSvgPathManager;
    return true;
}

GSvgPathManager::GSvgPathManager
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
    this->client()->imp()->svgPathManagerGlobals.emplace_back(this);
}

GSvgPathManager::~GSvgPathManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->svgPathManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GSvgPathManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GSvgPathManager::get_svg_path(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    new RSvgPath(
        static_cast<GSvgPathManager*>(wl_resource_get_user_data(resource)),
        id,
        wl_resource_get_version(resource));
}
