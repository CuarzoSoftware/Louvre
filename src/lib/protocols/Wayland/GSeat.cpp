#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>

#include <private/LClientPrivate.h>

#include <LCompositor.h>

GSeat::GSeat(LCompositor *compositor,
                             wl_client *client,
                             const wl_interface *interface,
                             Int32 version,
                             UInt32 id,
                             const void *implementation,
                             wl_resource_destroy_func_t destroy) :
    LResource(compositor,
              client,
              interface,
              version,
              id,
              implementation,
              destroy)
{
    m_imp = new GSeatPrivate();
    this->client()->imp()->seatGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->seatGlobals.end());
    sendCapabilities(this->compositor()->seat()->capabilities());
    sendName("seat0");
}

GSeat::~GSeat()
{
    client()->imp()->seatGlobals.erase(imp()->clientLink);

    if (keyboardResource())
        keyboardResource()->imp()->seatGlobal = nullptr;

    if (pointerResource())
        pointerResource()->imp()->seatGlobal = nullptr;

    if (dataDeviceResource())
        dataDeviceResource()->imp()->seatGlobal = nullptr;

    delete m_imp;
}

void GSeat::sendCapabilities(UInt32 capabilities)
{
    wl_seat_send_capabilities(resource(), capabilities);
}

void GSeat::sendName(const char *name)
{
#if LOUVRE_SEAT_VERSION >= WL_SEAT_NAME_SINCE_VERSION
    if (version() >= WL_SEAT_NAME_SINCE_VERSION)
        wl_seat_send_name(resource(), name);
#endif
}

RKeyboard *GSeat::keyboardResource() const
{
    return imp()->keyboardResource;
}

RPointer *GSeat::pointerResource() const
{
    return imp()->pointerResource;
}

RDataDevice *GSeat::dataDeviceResource() const
{
    return imp()->dataDeviceResource;
}
