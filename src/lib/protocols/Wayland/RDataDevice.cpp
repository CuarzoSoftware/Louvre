#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LCompositorPrivate.h>
#include <LClipboard.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_device_interface dataDevice_implementation =
{
    .start_drag = &RDataDevice::RDataDevicePrivate::start_drag,
    .set_selection = &RDataDevice::RDataDevicePrivate::set_selection,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
    .release = &RDataDevice::RDataDevicePrivate::release
#endif
};

RDataDevice::RDataDevice
(
    GDataDeviceManager *gDataDeviceManager,
    GSeat *gSeat,
    Int32 id
)
    :LResource
    (
        gSeat->client(),
        &wl_data_device_interface,
        gDataDeviceManager->version(),
        id,
        &dataDevice_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RDataDevice)
{
    imp()->gSeat = gSeat;
    gSeat->imp()->rDataDevice = this;
}

RDataDevice::~RDataDevice()
{
    if (seatGlobal())
        seatGlobal()->imp()->rDataDevice = nullptr;
}

GSeat *RDataDevice::seatGlobal() const
{
    return imp()->gSeat;
}

const RDataDevice::LastEventSerials &RDataDevice::serials() const
{
    return imp()->serials;
}

RDataOffer *RDataDevice::createOffer(RDataSource::Usage usage) noexcept
{
    LClipboard &clipboard { *seat()->clipboard() };

    if (usage == RDataSource::Clipboard)
    {
        if (clipboard.mimeTypes().empty())
        {
            selection(nullptr);
            return nullptr;
        }

        RDataOffer *offer { new RDataOffer(this, 0, RDataSource::Clipboard) };
        clipboard.m_dataOffer.reset(offer);
        dataOffer(offer);

        for (const auto &mimeType : seat()->clipboard()->mimeTypes())
            offer->offer(mimeType.mimeType.c_str());

        selection(offer);

        return offer;
    }

    return nullptr;
}

bool RDataDevice::dataOffer(RDataOffer *offer)
{
    wl_data_device_send_data_offer(resource(), offer->resource());
    return true;
}

bool RDataDevice::enter(UInt32 serial, RSurface *surface, Float24 x, Float24 y, RDataOffer *id)
{
    wl_data_device_send_enter(resource(), serial, surface->resource(), x, y, id->resource());
    return true;
}

bool RDataDevice::leave()
{
    wl_data_device_send_leave(resource());
    return true;
}

bool RDataDevice::motion(UInt32 time, Float24 x, Float24 y)
{
    wl_data_device_send_motion(resource(), time, x, y);
    return true;
}

bool RDataDevice::drop()
{
    wl_data_device_send_drop(resource());
    return true;
}

bool RDataDevice::selection(RDataOffer *offer)
{
    if (offer)
        wl_data_device_send_selection(resource(), offer->resource());
    else
        wl_data_device_send_selection(resource(), NULL);

    return true;
}
