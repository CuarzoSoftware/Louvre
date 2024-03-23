#ifndef LCLIPBOARD_H
#define LCLIPBOARD_H

#include <LObject.h>
#include <LWeak.h>

/**
 * @brief Clipboard manager.
 *
 * LClipboard lets you manage which clients can set the clipboard
 * and also specify which MIME types to make persistent.
 *
 * Clients can access the clipboard when one of their surfaces acquires keyboard focus.
 */
class Louvre::LClipboard : public LObject
{
public:

    /**
     * @brief Structure representing a Clipboard MIME type.
     */
    struct MimeTypeFile
    {
        std::string mimeType; /**< Mime type string. */
        FILE *tmp { NULL }; /**< Clipboard content for the MIME type (can be NULL). */
    };

    /**
     * @brief Constructor.
     *
     * @see LCompositor::createClipboardRequest().
     */
    LClipboard(const void *params);

    /**
     * @brief Desructor.
     */
    ~LClipboard()
    {
        clear();
    }

    /// @cond OMIT
    LCLASS_NO_COPY(LClipboard)
    /// @endcond

    /**
     * @brief Client request to set the clipboard.
     *
     * Returning `true` grants the client permission to set the clipboard and `false` prohibits it.\n
     * Only a single client can set the clipboard at a time.
     *
     * #### Default Implementation
     * @snippet LClipboardDefault.cpp setClipboardRequest
     */
    virtual bool setClipboardRequest(LClient *client, const LEvent &triggeringEvent);

    /**
     * @brief Filter of persistent clipboard MIME types.
     *
     * Keep the clipboard data for specific MIME types even after the
     * client owning the clipboard data is disconnected.
     *
     * @return `true` to make the MIME type persistent, `false` otherwise.
     *
     * #### Default Implementation
     * @snippet LClipboardDefault.cpp persistentMimeTypeFilter
     */
    virtual bool persistentMimeTypeFilter(const std::string &mimeType) const ;

    /**
     * @brief Access to the current clipboard MIME types.
     */
    const std::vector<MimeTypeFile> &mimeTypes() const noexcept;

    /// @cond OMIT
private:
    friend class Protocols::Wayland::RDataDevice;
    friend class Protocols::Wayland::RDataSource;
    friend class Protocols::Wayland::RDataOffer;
    friend class LKeyboard;
    LWeak<Protocols::Wayland::RDataSource> m_dataSource;
    LWeak<Protocols::Wayland::RDataOffer> m_dataOffer;
    std::vector<MimeTypeFile> m_persistentMimeTypes;
    void clear() noexcept;
    /// @endcond
};

#endif // LCLIPBOARD_H
