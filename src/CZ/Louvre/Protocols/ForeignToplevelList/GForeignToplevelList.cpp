#include <CZ/Louvre/Protocols/ForeignToplevelList/ext-foreign-toplevel-list-v1.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LToplevelRole.h>
#include <LCompositor.h>
#include <LUtils.h>

using namespace Louvre;
using namespace Louvre::Protocols::ForeignToplevelList;

static const struct ext_foreign_toplevel_list_v1_interface imp
{
    .stop = &GForeignToplevelList::stop,
    .destroy = &GForeignToplevelList::destroy
};

void GForeignToplevelList::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GForeignToplevelList(client, version, id);
}

Int32 GForeignToplevelList::maxVersion() noexcept
{
    return LOUVRE_FOREIGN_TOPLEVEL_LIST_VERSION;
}

const wl_interface *GForeignToplevelList::interface() noexcept
{
    return &ext_foreign_toplevel_list_v1_interface;
}

GForeignToplevelList::GForeignToplevelList
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
    this->client()->imp()->foreignToplevelListGlobals.emplace_back(this);

    for (LSurface *s : compositor()->surfaces())
        if (s->toplevel() && s->mapped())
            toplevel(*s->toplevel());
}

GForeignToplevelList::~GForeignToplevelList() noexcept
{
    if (!m_finished)
        LVectorRemoveOneUnordered(client()->imp()->foreignToplevelListGlobals, this);
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
