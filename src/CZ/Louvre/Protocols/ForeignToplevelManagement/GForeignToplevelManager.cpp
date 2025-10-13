#include <CZ/Louvre/Protocols/ForeignToplevelManagement/wlr-foreign-toplevel-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::ForeignToplevelManagement;
using namespace CZ;

static const struct zwlr_foreign_toplevel_manager_v1_interface imp
{
    .stop = &GForeignToplevelManager::stop
};

LGLOBAL_INTERFACE_IMP(GForeignToplevelManager, LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION, zwlr_foreign_toplevel_manager_v1_interface)

bool GForeignToplevelManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.WlrForeignToplevelManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlrForeignToplevelManager;
    return true;
}

GForeignToplevelManager::GForeignToplevelManager
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
    this->client()->imp()->foreignToplevelManagerGlobals.emplace_back(this);

    for (LSurface *s : compositor()->surfaces())
        if (s->toplevel() && !s->toplevel()->m_flags.has(LToplevelRole::HasPendingFirstMap))
            toplevel(*s->toplevel());
}

GForeignToplevelManager::~GForeignToplevelManager() noexcept
{
    if (!m_finished)
        CZVectorUtils::RemoveOneUnordered(client()->imp()->foreignToplevelManagerGlobals, this);
}

/******************** EVENTS ********************/

void GForeignToplevelManager::toplevel(LToplevelRole &toplevelRole)
{
    if (m_stopped || !toplevelRole.foreignControllerFilter(this))
        return;

    new RForeignToplevelHandle(*this, toplevelRole);
}

const std::vector<GForeignToplevelManager*>::iterator GForeignToplevelManager::finished()
{
    if (m_finished)
        return client()->imp()->foreignToplevelManagerGlobals.end();

    m_finished = true;
    zwlr_foreign_toplevel_manager_v1_send_finished(resource());

    auto it { client()->imp()->foreignToplevelManagerGlobals.erase(
            std::find(
                client()->imp()->foreignToplevelManagerGlobals.begin(),
                client()->imp()->foreignToplevelManagerGlobals.end(),
                this)) };

    wl_resource_destroy(resource());

    return it;
}

/******************** REQUESTS ********************/

void GForeignToplevelManager::stop(wl_client */*client*/, wl_resource *resource) noexcept
{
    auto &res { *static_cast<GForeignToplevelManager*>(wl_resource_get_user_data(resource)) };
    res.m_stopped = true;
}
