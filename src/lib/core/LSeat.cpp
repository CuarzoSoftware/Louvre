#include <private/LSeatPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>

#include <protocols/Wayland/LWaylandSeatGlobal.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <libinput.h>
#include <signal.h>

#include <xkbcommon/xkbcommon-compat.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon-names.h>
#include <sys/mman.h>

#include <sys/poll.h>
#include <sys/eventfd.h>

#include <LWayland.h>
#include <LCursor.h>
#include <LToplevelRole.h>
#include <LSurface.h>
#include <LTime.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LLog.h>


using namespace Louvre;

void initSeat(LSeat *seat)
{
    seat->imp()->listener.enable_seat = &LSeat::LSeatPrivate::seatEnabled;
    seat->imp()->listener.disable_seat = &LSeat::LSeatPrivate::seatDisabled;

    seat->imp()->libseatHandle = libseat_open_seat(&seat->imp()->listener, seat);

    seat->imp()->dispatchSeat();

    LWayland::addFdListener(
                libseat_get_fd(seat->libseatHandle()),
                seat,
                &LSeat::LSeatPrivate::seatEvent, POLLIN);
}
LSeat::LSeat(Louvre::LSeat::Params *params)
{
    m_imp = new LSeatPrivate();

    LDNDManager::Params dndManagerParams;
    dndManagerParams.seat = this;
    imp()->dndManager = params->compositor->createDNDManagerRequest(&dndManagerParams);
    imp()->compositor = params->compositor;

    LPointer::Params pointerParams;
    pointerParams.seat = this;
    imp()->pointer = params->compositor->createPointerRequest(&pointerParams);

    LKeyboard::Params keyboardParams;
    keyboardParams.seat = this;
    imp()->keyboard = params->compositor->createKeyboardRequest(&keyboardParams);

    imp()->enabled = true;
}

LSeat::~LSeat()
{
    delete m_imp;
}


LCompositor *LSeat::compositor() const
{
    return imp()->compositor;
}

LCursor *LSeat::cursor() const
{
    return compositor()->cursor();
}

UInt32 LSeat::backendCapabilities() const
{
    return compositor()->imp()->inputBackend->getCapabilities(this);
}

void *LSeat::backendContextHandle() const
{
    return compositor()->imp()->inputBackend->getContextHandle(this);
}

UInt32 LSeat::capabilities() const
{
    return imp()->capabilities;
}

void LSeat::setCapabilities(UInt32 capabilitiesFlags)
{
    imp()->capabilities = capabilitiesFlags;

    for(LClient *c : compositor()->clients())
    {
        for(LWaylandSeatGlobal *s : (list<LWaylandSeatGlobal*>&)c->seatGlobals())
            s->sendCapabilities(capabilitiesFlags);
    }
}

LToplevelRole *LSeat::activeToplevel() const
{
    return imp()->activeToplevel;
}


LPointer *LSeat::pointer() const
{
    return imp()->pointer;
}

LKeyboard *LSeat::keyboard() const
{
    return imp()->keyboard;
}

LDataSource *LSeat::dataSelection() const
{
    return imp()->dataSelection;
}

LDNDManager *LSeat::dndManager() const
{
    return imp()->dndManager;
}

Int32 LSeat::setTTY(Int32 tty)
{
    if(imp()->libseatHandle)
    {
        Int32 ret = libseat_switch_session(libseatHandle(), tty);
        libseat_dispatch(libseatHandle(), -1);
        return ret;
    }

    return 0;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd)
{
    if(!imp()->libseatHandle)
        initSeat(this);
    return libseat_open_device(libseatHandle(), path, fd);
}

Int32 LSeat::closeDevice(Int32 id)
{
    return libseat_close_device(libseatHandle(), id);
}


libseat *LSeat::libseatHandle() const
{
    return imp()->libseatHandle;
}

bool LSeat::enabled() const
{
    return imp()->enabled;
}


int LSeat::LSeatPrivate::seatEvent(int, unsigned int, void *data)
{
    LSeat *seat = (LSeat*)data;
    libseat_dispatch(seat->libseatHandle(), 0);
    return 0;
}

bool outputsWithPendingState(LCompositor *compositor, LOutput::State state)
{
    for(LOutput *o : compositor->outputs())
        if(o->state() != state)
            return false;

    return true;
}

void LSeat::LSeatPrivate::seatEnabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    lseat->imp()->enabled = true;
    lseat->compositor()->repaintAllOutputs();

    if(lseat->compositor()->inputBackendInitialized())
        lseat->compositor()->imp()->inputBackend->resume(lseat);

    LLog::debug("Seat %s enabled.", libseat_seat_name(seat));

    // Notifica
    lseat->seatEnabled();
}

void LSeat::LSeatPrivate::seatDisabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    if(!lseat->imp()->enabled)
        return;

    lseat->imp()->enabled = false;

    if(lseat->compositor()->inputBackendInitialized())
        lseat->compositor()->imp()->inputBackend->suspend(lseat);

    libseat_disable_seat(seat);

    for(LOutput *o : lseat->compositor()->outputs())
    {
        o->imp()->state = LOutput::Suspended;
    }

    LLog::debug("Seat %s disabled.", libseat_seat_name(seat));

    // Notifica
    lseat->seatDisabled();
}

void LSeat::LSeatPrivate::dispatchSeat()
{
    if(libseatHandle)
        libseat_dispatch(libseatHandle, 0);
}


LSeat::LSeatPrivate *LSeat::imp() const
{
    return m_imp;
}


