#include <protocols/DRMLease/drm-lease-v1.h>
#include <protocols/DRMLease/RDRMLease.h>
#include <protocols/DRMLease/RDRMLeaseRequest.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <LUtils.h>
#include <LOutput.h>

using namespace Louvre::Protocols::DRMLease;

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
            m_leasedConnectors.emplace_back(output);
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
    for (std::size_t i = 0; i < m_leasedConnectors.size(); i++)
    {
        if (!m_leasedConnectors[i]->leasable())
        {
            m_leasedConnectors[i] = m_leasedConnectors.back();
            m_leasedConnectors.pop_back();
        }
    }

    const int fd { compositor()->imp()->graphicBackend->backendCreateLease(m_leasedConnectors) };

    if (fd < 0)
    {
        finished();
        return;
    }

    sendFd(fd);

    for (LOutput *output : m_leasedConnectors)
        output->imp()->lease.reset(this);

    // TODO: Notify the lease was created ?
}

RDRMLease::~RDRMLease()
{
    if (m_fd < 0)
        return;

    compositor()->imp()->graphicBackend->backendRevokeLease(m_fd);
}

/******************** REQUESTS ********************/

void RDRMLease::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RDRMLease::sendFd(int fd)
{
    if (m_finished || m_fd >= 0)
        return;

    wp_drm_lease_v1_send_lease_fd(resource(), fd);
    close(fd);
    m_fd = fd; // Used as ID in the backend
}

void RDRMLease::finished()
{
    if (m_finished)
        return;

    wp_drm_lease_v1_send_finished(resource());

    if (m_fd >= 0)
    {
        compositor()->imp()->graphicBackend->backendRevokeLease(m_fd);
        m_fd = -1;
    }
    m_finished = true;
    notifyDestruction();
}
