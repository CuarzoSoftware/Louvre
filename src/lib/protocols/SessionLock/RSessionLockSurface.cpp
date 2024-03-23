#include <protocols/SessionLock/GSessionLockManager.h>
#include <protocols/SessionLock/RSessionLockSurface.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <private/LSurfacePrivate.h>
#include <LSessionLockRole.h>
#include <LCompositor.h>
#include <LOutput.h>

using namespace Louvre::Protocols::SessionLock;

static const struct ext_session_lock_surface_v1_interface imp
{
    .destroy = &RSessionLockSurface::destroy,
    .ack_configure = &RSessionLockSurface::ack_configure
};

RSessionLockSurface::RSessionLockSurface(RSessionLock *sessionLockRes, LSurface *surface, LOutput *output, UInt32 id) :
    LResource
    (
        sessionLockRes->client(),
        &ext_session_lock_surface_v1_interface,
        sessionLockRes->version(),
        id,
        &imp
    ),
    m_sessionLockRes(sessionLockRes)
{
    LSessionLockRole::Params params
    {
        this,
        surface,
        output
    };

    m_sessionLockRole.reset(compositor()->createSessionLockRoleRequest(&params));
    sessionLockRes->m_roles.push_back(sessionLockRole());
    surface->imp()->setPendingRole(sessionLockRole());
    surface->imp()->applyPendingRole();
    sessionLockRole()->configure(output->size());
}

RSessionLockSurface::~RSessionLockSurface()
{
    compositor()->destroySessionLockRoleRequest(sessionLockRole());

    if (sessionLockRole()->surface())
        sessionLockRole()->surface()->imp()->setMapped(false);

    if (sessionLockRes())
        LVectorRemoveOneUnordered(sessionLockRes()->m_roles, sessionLockRole());
}

void RSessionLockSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RSessionLockSurface::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &res { *static_cast<RSessionLockSurface*>(wl_resource_get_user_data(resource)) };

    while (!res.sessionLockRole()->m_sentConfs.empty())
    {
        if (res.sessionLockRole()->m_sentConfs.front().serial == serial)
        {
            res.sessionLockRole()->m_currentSize = res.sessionLockRole()->m_sentConfs.front().size;
            res.sessionLockRole()->m_sentConfs.pop();
            return;
        }
        else
            res.sessionLockRole()->m_sentConfs.pop();
    }

    wl_resource_post_error(resource, EXT_SESSION_LOCK_SURFACE_V1_ERROR_INVALID_SERIAL, "Serial provided in ack_configure is invalid.");
}

void RSessionLockSurface::configure(UInt32 serial, UInt32 width, UInt32 height) noexcept
{
    ext_session_lock_surface_v1_send_configure(resource(), serial, width, height);
}
