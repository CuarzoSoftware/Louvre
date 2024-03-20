#ifndef RDATAOFFER_H
#define RDATAOFFER_H

#include <protocols/Wayland/RDataSource.h>

class Louvre::Protocols::Wayland::RDataOffer final : public LResource
{
public:

    RDataDevice *dataDeviceRes() const noexcept
    {
        return m_dataDeviceRes.get();
    }

    RDataSource::Usage usage() const noexcept
    {
        return m_usage;
    }

    /******************** DND ONLY ********************/

    UInt32 actions() const noexcept
    {
        return m_actions;
    }

    UInt32 preferredAction() const noexcept
    {
        return m_preferredAction;
    }

    bool matchedMimeType() const noexcept
    {
        return m_matchedMimeType;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type) noexcept;
    static void receive(wl_client *client, wl_resource *resource, const char *mime_type, Int32 fd) noexcept;

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    static void finish(wl_client *client, wl_resource *resource) noexcept;
    static void set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void offer(const char *mimeType) noexcept;

    // Since 3
    bool sourceActions(UInt32 sourceActions) noexcept;
    bool action(UInt32 dndAction) noexcept;

private:
    friend class Louvre::Protocols::Wayland::RDataDevice;
    friend class Louvre::LDND;
    RDataOffer(RDataDevice *dataDeviceRes, UInt32 id, RDataSource::Usage usage) noexcept;
    ~RDataOffer() noexcept;
    LWeak<RDataDevice> m_dataDeviceRes;
    RDataSource::Usage m_usage;
    std::shared_ptr<LDNDSession> m_dndSession;
    UInt32 m_actions { 0 };
    UInt32 m_preferredAction { 0 };
    bool m_matchedMimeType { false };
};

#endif // RDATAOFFER_H
