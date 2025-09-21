#include <LSurface.h>
#include <protocols/SvgPath/GSvgPathManager.h>
#include <protocols/SvgPath/RSvgPath.h>
#include <protocols/SvgPath/lvr-svg-path.h>

using namespace Louvre::Protocols::SvgPath;

static const struct lvr_svg_path_interface imp{
    .destroy = &RSvgPath::destroy,
    .concat_commands = &RSvgPath::concat_commands,
    .done = &RSvgPath::done,
};

RSvgPath::RSvgPath(GSvgPathManager *manager, UInt32 id, Int32 version) noexcept
    : LResource(manager->client(), &lvr_svg_path_interface, version, id, &imp) {
}

/******************** REQUESTS ********************/

void RSvgPath::destroy(wl_client * /*client*/, wl_resource *resource) {
  auto &res{*static_cast<RSvgPath *>(wl_resource_get_user_data(resource))};

  if (!res.isComplete()) {
    res.postError(LVR_SVG_PATH_ERROR_INCOMPLETE, "incomplete svg path");
    return;
  }

  wl_resource_destroy(resource);
}

void RSvgPath::concat_commands(wl_client * /*client*/, wl_resource *resource,
                               const char *commands) {
  auto &res{*static_cast<RSvgPath *>(wl_resource_get_user_data(resource))};

  if (res.isComplete()) {
    res.postError(LVR_SVG_PATH_ERROR_ALREADY_CONSTRUCTED, "done already sent");
    return;
  }

  res.m_commands += commands;
}

void RSvgPath::done(wl_client * /*client*/, wl_resource *resource) {
  auto &res{*static_cast<RSvgPath *>(wl_resource_get_user_data(resource))};

  if (res.isComplete()) {
    res.postError(LVR_SVG_PATH_ERROR_ALREADY_CONSTRUCTED,
                  "a request other than destroy was made after a done request");
    return;
  }

  res.m_isComplete = true;
}
