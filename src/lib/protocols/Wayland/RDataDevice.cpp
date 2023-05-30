#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LSeatPrivate.h>

#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataOffer.h>

#include <LDataOffer.h>
#include <LKeyboard.h>
#include <LPointer.h>
#include <LTime.h>

#include <stdio.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_device_interface dataDevice_implementation =
{
    .start_drag = &RDataDevice::RDataDevicePrivate::start_drag,
    .set_selection = &RDataDevice::RDataDevicePrivate::set_selection,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= WL_DATA_DEVICE_RELEASE_SINCE_VERSION
    .release = &RDataDevice::RDataDevicePrivate::release
#endif
};

RDataDevice::RDataDevice
(
    GDataDeviceManager *dataDeviceManagerGlobal,
    GSeat *seatGlobal,
    Int32 id
)
    :LResource
    (
        seatGlobal->client(),
        &wl_data_device_interface,
        dataDeviceManagerGlobal->version(),
        id,
        &dataDevice_implementation,
        &RDataDevice::RDataDevicePrivate::resource_destroy
    )
{
    m_imp = new RDataDevicePrivate();
    seatGlobal->imp()->dataDeviceResource = this;
}

RDataDevice::~RDataDevice()
{
    if (seatGlobal())
        seatGlobal()->imp()->dataDeviceResource = nullptr;

    delete m_imp;
}

void RDataDevice::sendEnter(LSurface *surface, Float64 x, Float64 y, RDataOffer *dataOfferResource)
{
    wl_data_device_send_enter(resource(),
                             LCompositor::nextSerial(),
                             surface->surfaceResource()->resource(),
                             wl_fixed_from_double(x),
                             wl_fixed_from_double(y),
                             dataOfferResource->resource());
}

void RDataDevice::sendLeave()
{
    wl_data_device_send_leave(resource());
}

void RDataDevice::sendMotion(Float64 x, Float64 y)
{
    wl_data_device_send_motion(resource(),
                               LTime::ms(),
                               wl_fixed_from_double(x),
                               wl_fixed_from_double(y));
}

void RDataDevice::sendDrop()
{
    wl_data_device_send_drop(resource());
}

void RDataDevice::sendDataOffer(RDataOffer *dataOfferResource)
{
    wl_data_device_send_data_offer(resource(), dataOfferResource->resource());
}

void RDataDevice::sendSelection(RDataOffer *dataOfferResource)
{
    wl_data_device_send_selection(resource(), dataOfferResource->resource());
}

GSeat *RDataDevice::seatGlobal() const
{
    return imp()->seatGlobal;
}

LDataOffer *RDataDevice::dataOffered() const
{
    return imp()->dataOffered;
}
