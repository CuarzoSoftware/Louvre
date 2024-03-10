#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/RTouchPrivate.h>
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
    const void *implementation
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation
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
        imp()->keyboardResources.pop_back();
    }

    while (!pointerResources().empty())
    {
        pointerResources().back()->imp()->gSeat = nullptr;
        imp()->pointerResources.pop_back();
    }

    while (!touchResources().empty())
    {
        touchResources().back()->imp()->gSeat = nullptr;
        imp()->touchResources.pop_back();
    }
}

const std::vector<RKeyboard *> &GSeat::keyboardResources() const
{
    return imp()->keyboardResources;
}

const std::vector<RPointer*> &GSeat::pointerResources() const
{
    return imp()->pointerResources;
}

const std::vector<RTouch *> &GSeat::touchResources() const
{
    return imp()->touchResources;
}

RDataDevice *GSeat::dataDeviceResource() const
{
    return imp()->rDataDevice.get();
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
