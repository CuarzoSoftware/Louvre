#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfiguration.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LSeat.h>
#include <LUtils.h>

using namespace Louvre;
using namespace Louvre::Protocols::WlrOutputManagement;

static const struct zwlr_output_manager_v1_interface imp
{
    .create_configuration = &GWlrOutputManager::create_configuration,
    .stop = &GWlrOutputManager::stop
};

void GWlrOutputManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GWlrOutputManager(client, version, id);
}

Int32 GWlrOutputManager::maxVersion() noexcept
{
    return LOUVRE_WLR_OUTPUT_MANAGER_VERSION;
}

const wl_interface *GWlrOutputManager::interface() noexcept
{
    return &zwlr_output_manager_v1_interface;
}

GWlrOutputManager::GWlrOutputManager
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
    this->client()->imp()->wlrOutputManagerGlobals.emplace_back(this);

    for (LOutput *output : seat()->outputs())
        head(output);

    done();
}

GWlrOutputManager::~GWlrOutputManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->wlrOutputManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GWlrOutputManager::create_configuration(wl_client */*client*/, wl_resource *resource, UInt32 id, UInt32 serial)
{
    auto &res { LRES_CAST(GWlrOutputManager, resource) };
    new RWlrOutputConfiguration(&res, id, serial);
}

void GWlrOutputManager::stop(wl_client */*client*/, wl_resource *resource)
{
    auto &res { LRES_CAST(GWlrOutputManager, resource) };
    res.m_stopped = true;
}

/******************** EVENTS ********************/

RWlrOutputHead *GWlrOutputManager::head(LOutput *output) noexcept
{
    if (m_stopped)
        return nullptr;

    for (auto *head : m_heads)
        if (head->output() == output)
            return nullptr;

    return new RWlrOutputHead(this, output);
}

void GWlrOutputManager::done() noexcept
{
    if (!m_pendingDone)
        return;

    m_pendingDone = false;
    m_serial = LTime::nextSerial();
    zwlr_output_manager_v1_send_done(resource(), m_serial);
}

void GWlrOutputManager::finished() noexcept
{
    zwlr_output_manager_v1_send_finished(resource());
    destroy();
}
