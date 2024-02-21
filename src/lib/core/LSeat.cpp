#include <private/LSeatPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LTouchPrivate.h>
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

#include <poll.h>
#include <sys/eventfd.h>

#include <LCursor.h>
#include <LToplevelRole.h>
#include <LSurface.h>
#include <LTime.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LClient.h>
#include <LLog.h>

using namespace Louvre;

LSeat::LSeat(const void *params) : LPRIVATE_INIT_UNIQUE(LSeat)
{
    L_UNUSED(params);
    compositor()->imp()->seat = this;

    LDNDManager::Params dndManagerParams;
    imp()->dndManager = compositor()->createDNDManagerRequest(&dndManagerParams);

    LPointer::Params pointerParams;
    imp()->pointer = compositor()->createPointerRequest(&pointerParams);

    LKeyboard::Params keyboardParams;
    imp()->keyboard = compositor()->createKeyboardRequest(&keyboardParams);

    LTouch::Params touchParams;
    imp()->touch = compositor()->createTouchRequest(&touchParams);

    imp()->enabled = true;
}

LSeat::~LSeat()
{
    if (imp()->libseatHandle)
    {
        libseat_close_seat(imp()->libseatHandle);
        imp()->libseatHandle = nullptr;
    }}

const std::vector<LOutput *> &LSeat::outputs() const
{
    return *compositor()->imp()->graphicBackend->backendGetConnectedOutputs();
}

void *LSeat::graphicBackendContextHandle() const
{
    return compositor()->imp()->graphicBackend->backendGetContextHandle();
}

UInt32 LSeat::graphicBackendId() const
{
    return compositor()->imp()->graphicBackend->backendGetId();
}

LSeat::InputCapabilitiesFlags LSeat::inputBackendCapabilities() const
{
    return compositor()->imp()->inputBackend->backendGetCapabilities();
}

const char *LSeat::name() const
{
    if (imp()->libseatHandle)
        return libseat_seat_name(imp()->libseatHandle);

    return "seat0";
}

void *LSeat::inputBackendContextHandle() const
{
    return compositor()->imp()->inputBackend->backendGetContextHandle();
}

UInt32 LSeat::inputBackendId() const
{
    return compositor()->imp()->inputBackend->backendGetId();
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

LTouch *LSeat::touch() const
{
    return imp()->touch;
}

LDataSource *LSeat::dataSelection() const
{
    return imp()->dataSelection;
}

LDNDManager *LSeat::dndManager() const
{
    return imp()->dndManager;
}

void LSeat::setTTY(UInt32 tty)
{
    if (imp()->libseatHandle)
        imp()->ttyNumber = tty;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd)
{
    if (!imp()->libseatHandle)
        return -1;

    Int32 id = libseat_open_device(libseatHandle(), path, fd);

    if (id == -1)
        LLog::error("[LSeat::openDevice] Failed to open device %s, id %d, %d.", path, id, *fd);

    return id;
}

Int32 LSeat::closeDevice(Int32 id)
{
    if (!imp()->libseatHandle)
        return -1;

    Int32 ret = libseat_close_device(libseatHandle(), id);

    if (ret == -1)
        LLog::error("[LSeat::closeDevice] Failed to close device %d.", id);

    return ret;
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
