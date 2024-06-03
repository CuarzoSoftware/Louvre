#include <private/LSeatPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>

#include <protocols/Wayland/GSeat.h>

#include <unistd.h>
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
#include <cassert>

#include <LCursor.h>
#include <LToplevelRole.h>
#include <LSurface.h>
#include <LTime.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LClient.h>
#include <LLog.h>

#include <private/LFactory.h>

using namespace Louvre;

LSeat::LSeat(const void *params) : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LSeat)
{
    assert(params != nullptr && "Invalid parameter passed to LSeat constructor.");
    LSeat**ptr { (LSeat**) params };
    assert(*ptr == nullptr && *ptr == seat() && "Only a single LSeat instance can exist.");
    *ptr = this;

    LFactory::createObject<LDND>(&m_dnd);
    LFactory::createObject<LPointer>(&m_pointer);
    LFactory::createObject<LKeyboard>(&m_keyboard);
    LFactory::createObject<LTouch>(&m_touch);
    LFactory::createObject<LClipboard>(&m_clipboard);
    imp()->enabled = true;
}

LSeat::~LSeat()
{
    if (imp()->libseatHandle)
    {
        libseat_close_seat(imp()->libseatHandle);
        imp()->libseatHandle = nullptr;
    }}

const std::vector<LOutput *> &LSeat::outputs() const noexcept
{
    return *compositor()->imp()->graphicBackend->backendGetConnectedOutputs();
}

const char *LSeat::name() const noexcept
{
    if (imp()->libseatHandle)
        return libseat_seat_name(imp()->libseatHandle);

    return "seat0";
}

LToplevelRole *LSeat::activeToplevel() const noexcept
{
    return imp()->activeToplevel;
}

const std::vector<LToplevelResizeSession *> &LSeat::toplevelResizeSessions() const noexcept
{
    return imp()->resizeSessions;
}

const std::vector<LToplevelMoveSession *> &LSeat::toplevelMoveSessions() const noexcept
{
    return imp()->moveSessions;
}

void LSeat::dismissPopups() noexcept
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->dismiss();
    }
}

LPopupRole *LSeat::topmostPopup() const noexcept
{
    for (std::list<LSurface*>::const_reverse_iterator it = compositor()->surfaces().crbegin();
         it != compositor()->surfaces().crend(); it++)
    {
        if ((*it)->mapped() && (*it)->popup())
            return (*it)->popup();
    }

    return nullptr;
}

void LSeat::setTTY(UInt32 tty) noexcept
{
    if (imp()->libseatHandle)
        imp()->ttyNumber = tty;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd) noexcept
{
    if (!imp()->libseatHandle)
        return -1;

    Int32 id = libseat_open_device(libseatHandle(), path, fd);

    if (id == -1)
        LLog::error("[LSeat::openDevice] Failed to open device %s, id %d, %d.", path, id, *fd);

    return id;
}

Int32 LSeat::closeDevice(Int32 id) noexcept
{
    if (!imp()->libseatHandle)
        return -1;

    Int32 ret = libseat_close_device(libseatHandle(), id);

    if (ret == -1)
        LLog::error("[LSeat::closeDevice] Failed to close device %d.", id);

    return ret;
}

libseat *LSeat::libseatHandle() const noexcept
{
    return imp()->libseatHandle;
}

bool LSeat::enabled() const noexcept
{
    return imp()->enabled;
}
