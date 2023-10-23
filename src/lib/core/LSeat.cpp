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
    imp()->dndManager = compositor()->createDNDManagerRequest(&dndManagerParams);

    LPointer::Params pointerParams;
    imp()->pointer = compositor()->createPointerRequest(&pointerParams);

    LKeyboard::Params keyboardParams;
    imp()->keyboard = compositor()->createKeyboardRequest(&keyboardParams);

    imp()->enabled = true;
}

LSeat::~LSeat()
{
    if (imp()->libseatHandle)
    {
        libseat_close_seat(imp()->libseatHandle);
        imp()->libseatHandle = nullptr;
    }

    delete m_imp;
}

const std::list<LOutput *> &LSeat::outputs() const
{
    return *compositor()->imp()->graphicBackend->getConnectedOutputs();
}

void *LSeat::graphicBackendContextHandle() const
{
    return compositor()->imp()->graphicBackend->getContextHandle();
}

UInt32 LSeat::graphicBackendId() const
{
    return compositor()->imp()->graphicBackend->id();
}

LSeat::InputCapabilitiesFlags LSeat::inputBackendCapabilities() const
{
    return compositor()->imp()->inputBackend->getCapabilities();
}

const char *LSeat::name() const
{
    if (imp()->libseatHandle)
        return libseat_seat_name(imp()->libseatHandle);

    return "seat0";
}

void *LSeat::inputBackendContextHandle() const
{
    return compositor()->imp()->inputBackend->getContextHandle();
}

UInt32 LSeat::inputBackendId() const
{
    return compositor()->imp()->inputBackend->id();
}

LSeat::InputCapabilitiesFlags LSeat::inputCapabilities() const
{
    return imp()->capabilities;
}

void LSeat::setInputCapabilities(LSeat::InputCapabilitiesFlags capabilitiesFlags)
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
        compositor()->imp()->unlockPoll();
        int ret = libseat_switch_session(libseatHandle(), tty);

        if (ret == 0)
            imp()->dispatchSeat();

        return ret;
    }

    return -1;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd, Int32 flags)
{
    if (!imp()->libseatHandle)
    {
        *fd = open(path, flags);
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

LPopupRole *LSeat::topmostPopup() const
{
    for (std::list<LSurface*>::const_reverse_iterator it = compositor()->surfaces().crbegin();
         it != compositor()->surfaces().crend(); it++)
    {
        if ((*it)->mapped() && (*it)->popup())
            return (*it)->popup();
    }

    return nullptr;
}
