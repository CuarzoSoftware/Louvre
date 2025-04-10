#include <protocols/SvgPath/GSvgPathManager.h>
#include <protocols/SvgPath/RSvgPath.h>
#include <protocols/SvgPath/svg-path.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::SvgPath;

static const struct svg_path_manager_interface imp
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
    return &svg_path_manager_interface;
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
