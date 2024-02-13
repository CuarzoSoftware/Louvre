#ifndef LDATADEVICE_H
#define LDATADEVICE_H

#include <LObject.h>

/**
 * @brief Data exchanging device between clients
 *
 * @note This class is primarily intended for internal use by Louvre and may not be directly useful to you.
 *
 * The LDataDevice class represents the [wl_data_device](https://wayland.app/protocols/wayland#wl_data_device)
 * interface of the Wayland protocol and is used by clients to handle drag & drop sessions and share information
 * via clipboard (also known as selections).
 * Clients with a data device can request to assign the clipboard and start drag & drop sessions. For this, they create a data source (LDataSource)
 * that contains the information they want to share, the library then notifies other clients
 * the presence of such a data source when they acquire pointer or keyboard focus through a data offer (LDataOffer). Finally clients
 * can request to acquire such information, for example when the user presses `Ctrl + v` (paste).\n
 * The library internally is responsible for coordinating the exchange of information between data devices, but provides control over which client can set the
 * clipboard or start drag & drop sessions.\n
 *
 * @see @li LSeat::setSelectionRequest() to manage client-set clipboard requests.
 * @see @li LDNDManager::startDragRequest() for handling client start drag & drop requests.
 */
class Louvre::LDataDevice : public LObject
{
public:

    /// @cond OMIT
    LDataDevice(const LDataDevice&) = delete;
    LDataDevice& operator= (const LDataDevice&) = delete;
    /// @endcond

    /**
     * @brief Notifies the clipboard characteristics to a client.
     *
     * The clipboard in Wayland is named ***Selection***. This method sends the mime types of the clipboard content to the client using an LDataOffer.\n
     * The library automatically invokes this method when any of the client's surfaces acquires keyboard focus.
     */
    void sendSelectionEvent();

    /**
     * @brief Client that owns the data device.
     */
    LClient *client() const;

    LPRIVATE_IMP_UNIQUE(LDataDevice)

    /// @cond OMIT
    friend class Louvre::LClient;
    LDataDevice();
    ~LDataDevice();
    /// @endcond
};

#endif // LDATADEVICE_H
