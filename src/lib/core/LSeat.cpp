#include <private/LSeatPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>

#include <protocols/Wayland/GSeat.h>

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

#include <LCursor.h>
#include <LToplevelRole.h>
#include <LSurface.h>
#include <LTime.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LLog.h>

using namespace Louvre;

LSeat::LSeat(Params *params)
{
    L_UNUSED(params);

    m_imp = new LSeatPrivate();
    compositor()->imp()->seat = this;

    LDNDManager::Params dndManagerParams;
    dndManagerParams.seat = this;
    imp()->dndManager = compositor()->createDNDManagerRequest(&dndManagerParams);

    LPointer::Params pointerParams;
    imp()->pointer = compositor()->createPointerRequest(&pointerParams);

    LKeyboard::Params keyboardParams;
    imp()->keyboard = compositor()->createKeyboardRequest(&keyboardParams);

    imp()->enabled = true;
}

LSeat::~LSeat()
{
    delete m_imp;
}

const list<LOutput *> *LSeat::outputs() const
{
    return LCompositor::compositor()->imp()->graphicBackend->getConnectedOutputs(LCompositor::compositor());
}

UInt32 LSeat::backendCapabilities() const
{
    return compositor()->imp()->inputBackend->getCapabilities(this);
}

const char *LSeat::name() const
{
    if (imp()->libseatHandle)
        return libseat_seat_name(imp()->libseatHandle);

    return "seat0";
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

    for (LClient *c : compositor()->clients())
    {
        for (Wayland::GSeat *s : c->seatGlobals())
            s->capabilities(capabilitiesFlags);
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
    if (imp()->libseatHandle)
    {
        Int32 ret = libseat_switch_session(libseatHandle(), tty);
        return ret;
    }

    return 0;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd)
{
    if (!imp()->libseatHandle)
    {
        *fd = open(path, O_CLOEXEC | O_RDWR);
        return *fd;
    }

    return libseat_open_device(libseatHandle(), path, fd);
}

Int32 LSeat::closeDevice(Int32 id)
{
    if (!imp()->libseatHandle)
    {
        close(id);
        return 0;
    }

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
