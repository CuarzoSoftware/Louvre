#include <LCompositor.h>
#include <LOutput.h>
#include <LSessionLockRole.h>
#include <LUtils.h>
#include <private/LFactory.h>
#include <private/LSurfacePrivate.h>
#include <protocols/SessionLock/GSessionLockManager.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <protocols/SessionLock/RSessionLockSurface.h>
#include <protocols/SessionLock/ext-session-lock-v1.h>

using namespace Louvre::Protocols::SessionLock;

static const struct ext_session_lock_surface_v1_interface imp{
    .destroy = &RSessionLockSurface::destroy,
    .ack_configure = &RSessionLockSurface::ack_configure};

RSessionLockSurface::RSessionLockSurface(RSessionLock *sessionLockRes,
                                         LSurface *surface, LOutput *output,
                                         UInt32 id)
    : LResource(sessionLockRes->client(),
                &ext_session_lock_surface_v1_interface,
                sessionLockRes->version(), id, &imp),
      m_sessionLockRes(sessionLockRes) {
  LSessionLockRole::Params params{this, surface, output};

  m_sessionLockRole.reset(LFactory::createObject<LSessionLockRole>(&params));
  sessionLockRes->m_roles.push_back(sessionLockRole());
  surface->imp()->notifyRoleChange();
  sessionLockRole()->configure(output->size());
}

RSessionLockSurface::~RSessionLockSurface() {
  compositor()->onAnticipatedObjectDestruction(sessionLockRole());

  if (sessionLockRes())
    LVectorRemoveOneUnordered(sessionLockRes()->m_roles, sessionLockRole());

  sessionLockRole()->surface()->imp()->setMapped(false);
  sessionLockRole()->surface()->imp()->setRole(nullptr);
  sessionLockRole()->surface()->imp()->notifyRoleChange();
}

void RSessionLockSurface::destroy(wl_client * /*client*/,
                                  wl_resource *resource) {
  wl_resource_destroy(resource);
}

void RSessionLockSurface::ack_configure(wl_client * /*client*/,
                                        wl_resource *resource, UInt32 serial) {
  auto &res{
      *static_cast<RSessionLockSurface *>(wl_resource_get_user_data(resource))};

  while (!res.sessionLockRole()->m_sentConfs.empty()) {
    if (res.sessionLockRole()->m_sentConfs.front().serial == serial) {
      res.sessionLockRole()->m_currentSize =
          res.sessionLockRole()->m_sentConfs.front().size;
      res.sessionLockRole()->m_sentConfs.pop();
      return;
    } else
      res.sessionLockRole()->m_sentConfs.pop();
  }

  res.postError(EXT_SESSION_LOCK_SURFACE_V1_ERROR_INVALID_SERIAL,
                "Serial provided in ack_configure is invalid.");
}

void RSessionLockSurface::configure(UInt32 serial, UInt32 width,
                                    UInt32 height) noexcept {
  ext_session_lock_surface_v1_send_configure(resource(), serial, width, height);
}
