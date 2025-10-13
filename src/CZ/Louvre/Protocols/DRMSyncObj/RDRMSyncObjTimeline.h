#ifndef RDRMSYNCOBJTIMELINE_H
#define RDRMSYNCOBJTIMELINE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Ream/Ream.h>
#include <memory>

class CZ::Protocols::DRMSyncObj::RDRMSyncObjTimeline final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);

    std::shared_ptr<RDRMTimeline> timeline() const noexcept { return m_timeline; }
private:
    friend class GDRMSyncObjManager;
    RDRMSyncObjTimeline(std::shared_ptr<RDRMTimeline> &&timeline, LClient *client, Int32 version, UInt32 id);
    ~RDRMSyncObjTimeline() noexcept = default;
    std::shared_ptr<RDRMTimeline> m_timeline;
};

#endif // RDRMSYNCOBJTIMELINE_H
