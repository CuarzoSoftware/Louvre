#include <CZ/Louvre/Protocols/SvgPath/GSvgPathManager.h>
#include <CZ/Louvre/Protocols/SvgPath/RSvgPath.h>
#include <CZ/Louvre/Protocols/SvgPath/lvr-svg-path.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::SvgPath;

static const struct lvr_svg_path_manager_interface imp
{
    .destroy = &GSvgPathManager::destroy,
    .get_svg_path = &GSvgPathManager::get_svg_path
};

void GSvgPathManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSvgPathManager(client, version, id);
}

Int32 GSvgPathManager::maxVersion() noexcept
{
    return LOUVRE_SVG_PATH_MANAGER_VERSION;
}

const wl_interface *GSvgPathManager::interface() noexcept
{
    return &lvr_svg_path_manager_interface;
}

GSvgPathManager::GSvgPathManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->svgPathManagerGlobals.emplace_back(this);
}

GSvgPathManager::~GSvgPathManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->svgPathManagerGlobals, this);
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
