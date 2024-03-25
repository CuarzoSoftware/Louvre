#ifndef RDATASOURCE_H
#define RDATASOURCE_H

#include <LClipboard.h>
#include <LResource.h>
#include <memory>
#include <stdio.h>

class Louvre::Protocols::Wayland::RDataSource final : public LResource
{
public:

    enum Usage
    {
        Undefined,
        Clipboard,
        DND
    };

    void requestPersistentMimeType(LClipboard::MimeTypeFile &mimeType);
    Usage usage() const noexcept
    {
        return m_usage;
    }

    /******************** DND ONLY ********************/

    UInt32 actions() const noexcept
    {
        return m_actions;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void offer(wl_client *client, wl_resource *resource, const char *mime_type);
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    static void set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void target(const char *mimeType) noexcept;
    void send(const char *mimeType, Int32 fd) noexcept;
    void cancelled() noexcept;

    // Since 3
    bool dndDropPerformed() noexcept;
    bool dndFinished() noexcept;
    bool action(UInt32 dndAction) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GDataDeviceManager;
    friend class Louvre::Protocols::Wayland::RDataDevice;
    friend class Louvre::LClipboard;
    RDataSource(GDataDeviceManager *dataDeviceManagerRes, UInt32 id) noexcept;
    ~RDataSource() noexcept;

    Usage m_usage { Undefined };
    std::vector<LClipboard::MimeTypeFile> m_mimeTypes;

    /******************** DND ONLY ********************/

    UInt32 m_actions { 0 };
    std::shared_ptr<LDNDSession> m_dndSession;
};

#endif // RDATASOURCE_H
