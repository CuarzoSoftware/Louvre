#include <LUtils.h>
#include <private/LClientPrivate.h>
#include <protocols/IdleInhibit/GIdleInhibitManager.h>
#include <protocols/IdleInhibit/RIdleInhibitor.h>
#include <protocols/IdleInhibit/idle-inhibit-unstable-v1.h>
#include <protocols/Wayland/RSurface.h>

using namespace Louvre::Protocols::IdleInhibit;
using namespace Louvre;

static const struct zwp_idle_inhibit_manager_v1_interface imp{
    .destroy = &GIdleInhibitManager::destroy,
    .create_inhibitor = &GIdleInhibitManager::create_inhibitor};

void GIdleInhibitManager::bind(wl_client *client, void * /*data*/,
                               UInt32 version, UInt32 id) noexcept {
  new GIdleInhibitManager(client, version, id);
}

Int32 GIdleInhibitManager::maxVersion() noexcept {
  return LOUVRE_IDLE_INHIBIT_MANAGER_VERSION;
}

const wl_interface *GIdleInhibitManager::interface() noexcept {
  return &zwp_idle_inhibit_manager_v1_interface;
}

GIdleInhibitManager::GIdleInhibitManager(wl_client *client, Int32 version,
                                         UInt32 id) noexcept
    : LResource(client, interface(), version, id, &imp) {
  this->client()->imp()->idleInhibitManagerGlobals.emplace_back(this);
}

GIdleInhibitManager::~GIdleInhibitManager() noexcept {
  LVectorRemoveOneUnordered(client()->imp()->idleInhibitManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GIdleInhibitManager::destroy(wl_client * /*client*/,
                                  wl_resource *resource) noexcept {
  wl_resource_destroy(resource);
}

void GIdleInhibitManager::create_inhibitor(wl_client * /*client*/,
                                           wl_resource *resource, UInt32 id,
                                           wl_resource *surface) noexcept {
  new RIdleInhibitor(
      static_cast<Wayland::RSurface *>(wl_resource_get_user_data(surface))
          ->surface(),
      wl_resource_get_version(resource), id);
}
