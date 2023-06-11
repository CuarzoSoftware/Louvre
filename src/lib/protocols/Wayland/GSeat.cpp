#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>

GSeat::GSeat
(
    wl_client *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation,
        destroy
    )
{
    m_imp = new GSeatPrivate();
    this->client()->imp()->seatGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->seatGlobals.end());
    capabilities(seat()->capabilities());
    name(seat()->name());
}

GSeat::~GSeat()
{
    client()->imp()->seatGlobals.erase(imp()->clientLink);

    if (keyboardResource())
        keyboardResource()->imp()->gSeat = nullptr;

    if (pointerResource())
        pointerResource()->imp()->gSeat = nullptr;

    if (dataDeviceResource())
        dataDeviceResource()->imp()->gSeat = nullptr;

    delete m_imp;
}

RKeyboard *GSeat::keyboardResource() const
{
    return imp()->rKeyboard;
}

RPointer *GSeat::pointerResource() const
{
    return imp()->rPointer;
}

RDataDevice *GSeat::dataDeviceResource() const
{
    return imp()->rDataDevice;
}

bool GSeat::capabilities(UInt32 capabilities)
{
    wl_seat_send_capabilities(resource(), capabilities);
    return true;
}

bool GSeat::name(const char *name)
{
#if LOUVRE_WL_SEAT_VERSION >= 2
    if (version() >= 2)
    {
        wl_seat_send_name(resource(), name);
        return true;
    }
#endif
    L_UNUSED(name);
    return false;
}
