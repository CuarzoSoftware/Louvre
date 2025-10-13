#include "LBackend.h"
#include <CZ/Louvre/Protocols/DRMLease/drm-lease-v1.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLease.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseRequest.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/Seat/LOutput.h>

using namespace CZ::Protocols::DRMLease;

static const struct wp_drm_lease_v1_interface imp
{
    .destroy = &RDRMLease::destroy
};

RDRMLease::RDRMLease
    (
        RDRMLeaseRequest *leaseRequestRes,
        UInt32 id
    )
    :LResource
    (
        leaseRequestRes->client(),
        &wp_drm_lease_v1_interface,
        leaseRequestRes->version(),
        id,
        &imp
    )
{
    for (auto &output : leaseRequestRes->requestedConnectors())
    {
        if (!output.get() || !output->leasable())
            continue;

        if (output->leaseRequest(client()))
            m_leasedConnectors.emplace(output);
    }

    if (m_leasedConnectors.empty())
    {
        finished();
        return;
    }

    // Cancel existing leases / uninitialize if used by the compositor
    for (LOutput *output : m_leasedConnectors)
    {
        if (output->lease())
            output->lease()->finished();
        else if (output->state() != LOutput::Uninitialized)
            compositor()->removeOutput(output);
    }

    // Check if the user changed the leasable status again
    for (auto it = m_leasedConnectors.begin(); it != m_leasedConnectors.end();)
    {
        if (!(*it)->leasable())
            it = m_leasedConnectors.erase(it);
        else
            it++;
    }

    if (m_leasedConnectors.empty())
    {
        finished();
        return;
    }

    auto lease { compositor()->backend()->createLease(m_leasedConnectors) };

    if (!lease)
    {
        finished();
        return;
    }

    sendFd(lease->fd());
    m_lease = lease;

    CZWeak<RDRMLease> ref { this };

    for (LOutput *output : m_leasedConnectors)
    {
        output->imp()->lease.reset(this);

        output->imp()->lease.setOnDestroyCallback([output](auto) {
            output->leaseChanged();
        });

        output->leaseChanged();

        if (!ref)
            break; // The user called finished()
    }
}

RDRMLease::~RDRMLease()
{
    m_lease.reset();
}

/******************** REQUESTS ********************/

void RDRMLease::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RDRMLease::sendFd(int fd)
{
    if (m_finished || m_lease)
        return;

    wp_drm_lease_v1_send_lease_fd(resource(), fd);
}

void RDRMLease::finished()
{
    if (m_finished)
        return;

    wp_drm_lease_v1_send_finished(resource());

    m_lease.reset();
    m_finished = true;
    notifyDestruction();
}
