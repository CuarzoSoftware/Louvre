#ifndef RWLROUTPUTCONFIGURATION_H
#define RWLROUTPUTCONFIGURATION_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::WlrOutputManagement::RWlrOutputConfiguration final : public LResource
{
public:

    enum Reply
    {
        Failed,
        Succeeded,
        Cancelled
    };

    bool checkAlreadyConfigured(LOutput *output) noexcept;
    Reply validate() noexcept;

    /******************** REQUESTS ********************/

    static void enable_head(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *head);
    static void disable_head(wl_client *client, wl_resource *resource, wl_resource *head);
    static void apply(wl_client *client, wl_resource *resource);
    static void test(wl_client *client, wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void reply(Reply reply) noexcept;

private:
    friend class GWlrOutputManager;
    friend class RWlrOutputConfigurationHead;
    RWlrOutputConfiguration(GWlrOutputManager *wlrOutputManager, UInt32 id, UInt32 serial) noexcept;
    ~RWlrOutputConfiguration() noexcept;
    LWeak<GWlrOutputManager> m_wlrOutputManager;
    std::vector<RWlrOutputConfigurationHead*> m_enabled;
    std::vector<LWeak<LOutput>> m_disabled;
    UInt32 m_serial;
    bool m_replied { false };
};

#endif // RWLROUTPUTCONFIGURATION_H
