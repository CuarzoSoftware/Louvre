#include <protocols/Wayland/private/LWaylandKeyboardResourcePrivate.h>
#include <protocols/Wayland/private/LWaylandPointerResourcePrivate.h>
#include <protocols/Wayland/private/LWaylandSeatGlobalPrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>

using namespace Louvre::Globals;


LWaylandSeatGlobal::LWaylandSeatGlobal(LCompositor *compositor,
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
    m_imp = new LWaylandSeatGlobalPrivate();
    this->client()->imp()->seatGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->seatGlobals.end());
    sendCapabilities(this->compositor()->seat()->capabilities());
    sendName("seat0");
}

LWaylandSeatGlobal::~LWaylandSeatGlobal()
{
    client()->imp()->seatGlobals.erase(imp()->clientLink);

    for(LWaylandKeyboardResource * k : keyboardResources())
        k->imp()->seatGlobal = nullptr;

    for(LWaylandPointerResource * p : pointerResources())
        p->imp()->seatGlobal = nullptr;

    delete m_imp;
}

void LWaylandSeatGlobal::sendCapabilities(UInt32 capabilities)
{
    wl_seat_send_capabilities(resource(), capabilities);
}

void LWaylandSeatGlobal::sendName(const char *name)
{
#if LOUVRE_SEAT_VERSION >= WL_SEAT_NAME_SINCE_VERSION
    if(version() >= WL_SEAT_NAME_SINCE_VERSION)
        wl_seat_send_name(resource(), name);
#endif
}

const std::list<LWaylandKeyboardResource *> &LWaylandSeatGlobal::keyboardResources()
{
    return imp()->keyboardResources;
}

const LWaylandSeatGlobal::LastKeyboardEventSerials &LWaylandSeatGlobal::keyboardSerials() const
{
    return imp()->keyboardSerials;
}

const std::list<LWaylandPointerResource *> &LWaylandSeatGlobal::pointerResources()
{
    return imp()->pointerResources;
}

const LWaylandSeatGlobal::LastPointerEventSerials &LWaylandSeatGlobal::pointerSerials() const
{
    return imp()->pointerSerials;
}

LWaylandSeatGlobal::LWaylandSeatGlobalPrivate *LWaylandSeatGlobal::imp() const
{
    return m_imp;
}

