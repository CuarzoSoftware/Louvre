#include <CZ/Louvre/Protocols/ForeignToplevelList/ext-foreign-toplevel-list-v1.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::ForeignToplevelList;

static const struct ext_foreign_toplevel_list_v1_interface imp
{
    .stop = &GForeignToplevelList::stop,
    .destroy = &GForeignToplevelList::destroy
};

LGLOBAL_INTERFACE_IMP(GForeignToplevelList, LOUVRE_FOREIGN_TOPLEVEL_LIST_VERSION, ext_foreign_toplevel_list_v1_interface)

bool GForeignToplevelList::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.ForeignToplevelList)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.ForeignToplevelList;
    return true;
}

GForeignToplevelList::GForeignToplevelList
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
    this->client()->imp()->foreignToplevelListGlobals.emplace_back(this);

    for (LSurface *s : compositor()->surfaces())
        if (s->toplevel() && s->mapped())
            toplevel(*s->toplevel());
}

GForeignToplevelList::~GForeignToplevelList() noexcept
{
    if (!m_finished)
        CZVectorUtils::RemoveOneUnordered(client()->imp()->foreignToplevelListGlobals, this);
}

/******************** EVENTS ********************/

void GForeignToplevelList::toplevel(LToplevelRole &toplevelRole)
{
    if (m_stopped || m_finished || !toplevelRole.foreignHandleFilter(this))
        return;

    new RForeignToplevelHandle(*this, toplevelRole);
}

const std::vector<GForeignToplevelList*>::iterator GForeignToplevelList::finished()
{
    if (m_finished)
        return client()->imp()->foreignToplevelListGlobals.end();

    m_finished = true;
    ext_foreign_toplevel_list_v1_send_finished(resource());

    auto it { client()->imp()->foreignToplevelListGlobals.erase(
        std::find(
            client()->imp()->foreignToplevelListGlobals.begin(),
            client()->imp()->foreignToplevelListGlobals.end(),
            this)) };

    return it;
}

/******************** REQUESTS ********************/

void GForeignToplevelList::stop(wl_client */*client*/, wl_resource *resource) noexcept
{
    auto &res { *static_cast<GForeignToplevelList*>(wl_resource_get_user_data(resource)) };
    res.m_stopped = true;
}

void GForeignToplevelList::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
