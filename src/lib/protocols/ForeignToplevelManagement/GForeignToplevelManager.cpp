#include <protocols/ForeignToplevelManagement/wlr-foreign-toplevel-management-unstable-v1.h>
#include <protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <private/LClientPrivate.h>
#include <LToplevelRole.h>
#include <LCompositor.h>
#include <LUtils.h>

using namespace Louvre::Protocols::ForeignToplevelManagement;
using namespace Louvre;

static const struct zwlr_foreign_toplevel_manager_v1_interface imp
{
    .stop = &GForeignToplevelManager::stop
};

void GForeignToplevelManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GForeignToplevelManager(client, version, id);
}

Int32 GForeignToplevelManager::maxVersion() noexcept
{
    return LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION;
}

const wl_interface *GForeignToplevelManager::interface() noexcept
{
    return &zwlr_foreign_toplevel_manager_v1_interface;
}

GForeignToplevelManager::GForeignToplevelManager
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
    this->client()->imp()->foreignToplevelManagerGlobals.emplace_back(this);

    for (LSurface *s : compositor()->surfaces())
        if (s->toplevel() && !s->toplevel()->m_flags.check(LToplevelRole::HasPendingFirstMap))
            toplevel(*s->toplevel());
}

GForeignToplevelManager::~GForeignToplevelManager() noexcept
{
    if (!m_finished)
        LVectorRemoveOneUnordered(client()->imp()->foreignToplevelManagerGlobals, this);
}

/******************** EVENTS ********************/

void GForeignToplevelManager::toplevel(LToplevelRole &toplevelRole)
{
    if (m_stopped)
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
