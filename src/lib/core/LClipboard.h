#ifndef LCLIPBOARD_H
#define LCLIPBOARD_H

#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/RDataOffer.h>

class Louvre::LClipboard : public LObject
{
public:
    LClipboard(const void *params) noexcept;
    ~LClipboard() noexcept;

    /**
     * @brief Request to set the clipboard.
     *
     * Reimplement this virtual method if you want to control which clients can set the clipboard.\n
     * The default implementation allows clients to set the clipboard only if one of their surfaces has pointer or keyboard focus.\n
     *
     * Returning `true` grants the client permission to set the clipboard and `false` prohibits it.\n
     *
     * @param device Data device that makes the request.
     *
     * #### Default Implementation
     * @snippet LClipboardDefault.cpp setClipboardRequest
     */
    virtual bool setClipboardRequest(LClient *client, const LEvent *triggeringEvent) noexcept;

    // TODO: add doc and default imp
    virtual bool persistentMimeTypeFilter(const std::string &mimeType) const noexcept;
    const std::vector<Protocols::Wayland::RDataSource::MimeTypeFile> &mimeTypes() const noexcept;

private:
    friend class Protocols::Wayland::RDataDevice;
    friend class Protocols::Wayland::RDataSource;
    friend class Protocols::Wayland::RDataOffer;
    friend class LKeyboard;
    LWeak<Protocols::Wayland::RDataSource> m_dataSource;
    LWeak<Protocols::Wayland::RDataOffer> m_dataOffer;
    std::vector<Protocols::Wayland::RDataSource::MimeTypeFile> m_persistentMimeTypes;
    void clear();
};

#endif // LCLIPBOARD_H
