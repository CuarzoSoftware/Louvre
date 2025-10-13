#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfiguration.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::WlrOutputManagement;

static const struct zwlr_output_manager_v1_interface imp
{
    .create_configuration = &GWlrOutputManager::create_configuration,
    .stop = &GWlrOutputManager::stop
};

LGLOBAL_INTERFACE_IMP(GWlrOutputManager, LOUVRE_WLR_OUTPUT_MANAGER_VERSION, zwlr_output_manager_v1_interface)

bool GWlrOutputManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.WlrOutputManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlrOutputManager;
    return true;
}

GWlrOutputManager::GWlrOutputManager
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
    this->client()->imp()->wlrOutputManagerGlobals.emplace_back(this);

    for (LOutput *output : seat()->outputs())
        head(output);

    done();
}

GWlrOutputManager::~GWlrOutputManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->wlrOutputManagerGlobals, this);
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
    m_serial = CZTime::NextSerial();
    zwlr_output_manager_v1_send_done(resource(), m_serial);
}

void GWlrOutputManager::finished() noexcept
{
    zwlr_output_manager_v1_send_finished(resource());
    destroy();
}
