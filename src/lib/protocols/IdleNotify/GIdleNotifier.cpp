#include <LUtils.h>
#include <private/LClientPrivate.h>
#include <protocols/IdleNotify/GIdleNotifier.h>
#include <protocols/IdleNotify/RIdleNotification.h>
#include <protocols/IdleNotify/ext-idle-notify-v1.h>

using namespace Louvre::Protocols::IdleNotify;
using namespace Louvre;

static const struct ext_idle_notifier_v1_interface imp{
    .destroy = &GIdleNotifier::destroy,
    .get_idle_notification = &GIdleNotifier::get_idle_notification};

void GIdleNotifier::bind(wl_client *client, void * /*data*/, UInt32 version,
                         UInt32 id) noexcept {
  new GIdleNotifier(client, version, id);
}

Int32 GIdleNotifier::maxVersion() noexcept {
  return LOUVRE_IDLE_NOTIFIER_VERSION;
}

const wl_interface *GIdleNotifier::interface() noexcept {
  return &ext_idle_notifier_v1_interface;
}

GIdleNotifier::GIdleNotifier(wl_client *client, Int32 version,
                             UInt32 id) noexcept
    : LResource(client, interface(), version, id, &imp) {
  this->client()->imp()->idleNotifierGlobals.emplace_back(this);
}

GIdleNotifier::~GIdleNotifier() noexcept {
  LVectorRemoveOneUnordered(client()->imp()->idleNotifierGlobals, this);
}

/******************** REQUESTS ********************/

void GIdleNotifier::destroy(wl_client * /*client*/,
                            wl_resource *resource) noexcept {
  wl_resource_destroy(resource);
}

void GIdleNotifier::get_idle_notification(wl_client * /*client*/,
                                          wl_resource *resource, UInt32 id,
                                          UInt32 timeout,
                                          wl_resource * /*seat*/) noexcept {
  new RIdleNotification(
      static_cast<GIdleNotifier *>(wl_resource_get_user_data(resource))
          ->client(),
      wl_resource_get_version(resource), id, timeout);
}
