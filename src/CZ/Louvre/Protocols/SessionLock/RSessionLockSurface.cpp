#include <CZ/Louvre/Protocols/SessionLock/ext-session-lock-v1.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLockSurface.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::SessionLock;

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

    m_sessionLockRole.reset(LFactory::createObject<LSessionLockRole>(&params));
    sessionLockRes->m_roles.push_back(sessionLockRole());
    surface->imp()->notifyRoleChange();
    surface->imp()->setLayer(LLayerTop);
    sessionLockRole()->configure(output->size());
}

RSessionLockSurface::~RSessionLockSurface()
{
    compositor()->onAnticipatedObjectDestruction(sessionLockRole());

    if (sessionLockRes())
        CZVectorUtils::RemoveOneUnordered(sessionLockRes()->m_roles, sessionLockRole());

    sessionLockRole()->surface()->imp()->setMapped(false);
    sessionLockRole()->surface()->imp()->setRole(nullptr, true);
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
            res.sessionLockRole()->m_pending.serial = res.sessionLockRole()->m_sentConfs.front().serial;
            res.sessionLockRole()->m_pending.size = res.sessionLockRole()->m_sentConfs.front().size;
            res.sessionLockRole()->m_sentConfs.pop();
            return;
        }
        else
            res.sessionLockRole()->m_sentConfs.pop();
    }

    res.postError(EXT_SESSION_LOCK_SURFACE_V1_ERROR_INVALID_SERIAL, "Serial provided in ack_configure is invalid.");
}

void RSessionLockSurface::configure(UInt32 serial, UInt32 width, UInt32 height) noexcept
{
    ext_session_lock_surface_v1_send_configure(resource(), serial, width, height);
}
