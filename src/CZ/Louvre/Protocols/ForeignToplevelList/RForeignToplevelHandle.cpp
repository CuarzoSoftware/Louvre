#include <CZ/Louvre/Protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/ext-foreign-toplevel-list-v1.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>

using namespace CZ::Protocols::ForeignToplevelList;

static const struct ext_foreign_toplevel_handle_v1_interface imp
{
    .destroy = &RForeignToplevelHandle::destroy
};

RForeignToplevelHandle::RForeignToplevelHandle
    (
        GForeignToplevelList &foreignToplevelListRes,
        LToplevelRole &toplevelRole
    )
    :LResource
    (
        foreignToplevelListRes.client(),
        &ext_foreign_toplevel_handle_v1_interface,
        foreignToplevelListRes.version(),
        0,
        &imp
    ),
    m_foreignToplevelListRes(&foreignToplevelListRes),
    m_toplevelRole(&toplevelRole)
{
    m_toplevelRole->m_foreignToplevelHandles.emplace_back(this);
    ext_foreign_toplevel_list_v1_send_toplevel(foreignToplevelListRes.resource(), resource());
    identifier(m_toplevelRole->m_identifier);
    appId(m_toplevelRole->appId());
    title(m_toplevelRole->title());
    done();
}

RForeignToplevelHandle::~RForeignToplevelHandle()
{
    if (m_closed)
        return;

    if (m_toplevelRole)
        CZVectorUtils::RemoveOneUnordered(m_toplevelRole->m_foreignToplevelHandles, this);
}

bool RForeignToplevelHandle::canSendParams() const noexcept
{
    return !m_closed && m_toplevelRole && m_foreignToplevelListRes && !m_foreignToplevelListRes->finishedSent();
}

/******************** REQUESTS ********************/

void RForeignToplevelHandle::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RForeignToplevelHandle::closed() noexcept
{
    if (m_closed)
        return;

    m_closed = true;

    if (m_toplevelRole)
        CZVectorUtils::RemoveOneUnordered(m_toplevelRole->m_foreignToplevelHandles, this);

    ext_foreign_toplevel_handle_v1_send_closed(resource());
}

void RForeignToplevelHandle::done() noexcept
{
    if (canSendParams())
        ext_foreign_toplevel_handle_v1_send_done(resource());
}

void RForeignToplevelHandle::title(const std::string &title) noexcept
{
    if (canSendParams())
        ext_foreign_toplevel_handle_v1_send_title(resource(), title.c_str());
}

void RForeignToplevelHandle::appId(const std::string &appId) noexcept
{
    if (canSendParams())
        ext_foreign_toplevel_handle_v1_send_app_id(resource(), appId.c_str());
}

void RForeignToplevelHandle::identifier(const std::string &identifier) noexcept
{
    if (canSendParams())
        ext_foreign_toplevel_handle_v1_send_identifier(resource(), identifier.c_str());
}

