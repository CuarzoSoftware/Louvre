#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LSeat.h>

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
    ),
    LPRIVATE_INIT_UNIQUE(GSeat)
{
    this->client()->imp()->seatGlobals.push_back(this);
    capabilities(seat()->inputCapabilities());
    name(seat()->name());
}

GSeat::~GSeat()
{
    LVectorRemoveOneUnordered(client()->imp()->seatGlobals, this);

    while (!keyboardResources().empty())
    {
        keyboardResources().back()->imp()->gSeat = nullptr;
        imp()->rKeyboards.pop_back();
    }

    while (!pointerResources().empty())
    {
        pointerResources().back()->imp()->gSeat = nullptr;
        imp()->rPointers.pop_back();
    }

    if (dataDeviceResource())
        dataDeviceResource()->imp()->gSeat = nullptr;
}

const std::vector<RKeyboard *> &GSeat::keyboardResources() const
{
    return imp()->rKeyboards;
}

const std::vector<RPointer*> &GSeat::pointerResources() const
{
    return imp()->rPointers;
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
