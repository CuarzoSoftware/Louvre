#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LClient.h>

const LEvent *LClient::findEventBySerial(UInt32 serial) const noexcept
{
    if (imp()->events.keyboard.enter.serial() == serial)
        return &imp()->events.keyboard.enter;
    else if (imp()->events.keyboard.leave.serial() == serial)
        return &imp()->events.keyboard.leave;
    else if (imp()->events.keyboard.key.serial() == serial)
        return &imp()->events.keyboard.key;
    else if (imp()->events.keyboard.modifiers.serial() == serial)
        return &imp()->events.keyboard.modifiers;

    else if (imp()->events.pointer.enter.serial() == serial)
        return &imp()->events.pointer.enter;
    else if (imp()->events.pointer.leave.serial() == serial)
        return &imp()->events.pointer.leave;
    else if (imp()->events.pointer.button.serial() == serial)
        return &imp()->events.pointer.button;

    else if (imp()->events.pointer.swipeBegin.serial() == serial)
        return &imp()->events.pointer.swipeBegin;
    else if (imp()->events.pointer.swipeEnd.serial() == serial)
        return &imp()->events.pointer.swipeEnd;

    else if (imp()->events.pointer.pinchBegin.serial() == serial)
        return &imp()->events.pointer.pinchBegin;
    else if (imp()->events.pointer.pinchEnd.serial() == serial)
        return &imp()->events.pointer.pinchEnd;

    else if (imp()->events.pointer.holdBegin.serial() == serial)
        return &imp()->events.pointer.holdBegin;
    else if (imp()->events.pointer.holdEnd.serial() == serial)
        return &imp()->events.pointer.holdEnd;

    for (auto it = imp()->events.touch.down.begin(); it != imp()->events.touch.down.end(); it++)
        if (it->serial() == serial)
            return &(*it);

    for (auto it = imp()->events.touch.up.begin(); it != imp()->events.touch.up.end(); it++)
        if (it->serial() == serial)
            return &(*it);

    return nullptr;
}

const LClientCursor &LClient::lastCursorRequest() const noexcept
{
    return imp()->lastCursorRequest;
}

LClient::LClient(const void *params) noexcept : m_imp { std::make_unique<LClientPrivate>(this, ((Params*)params)->client)}
{
    dataDevice().imp()->client = this;
}

LClient::~LClient() noexcept {}

bool LClient::ping(UInt32 serial) const noexcept
{
    if (imp()->xdgWmBaseGlobals.empty())
        return false;

    imp()->xdgWmBaseGlobals.front()->ping(serial);
    return true;
}

wl_client *LClient::client() const noexcept
{
    return imp()->client;
}

LDataDevice &LClient::dataDevice() const noexcept
{
    return imp()->dataDevice;
}

const std::vector<LSurface *> &LClient::surfaces() const noexcept
{
    return imp()->surfaces;
}

void LClient::flush() noexcept
{
    wl_client_flush(client());
}

void LClient::destroy() noexcept
{
    wl_client_destroy(client());
}

const std::vector<Wayland::GOutput*> &LClient::outputGlobals() const noexcept
{
    return imp()->outputGlobals;
}

const std::vector<Wayland::GCompositor*> &LClient::compositorGlobals() const noexcept
{
    return imp()->compositorGlobals;
}

const std::vector<Wayland::GSubcompositor*> &LClient::subcompositorGlobals() const noexcept
{
    return imp()->subcompositorGlobals;
}

const std::vector<Wayland::GSeat*> &LClient::seatGlobals() const noexcept
{
    return imp()->seatGlobals;
}

const Wayland::GDataDeviceManager *LClient::dataDeviceManagerGlobal() const noexcept
{
    return imp()->dataDeviceManagerGlobal;
}

const std::vector<XdgShell::GXdgWmBase *> &LClient::xdgWmBaseGlobals() const noexcept
{
    return imp()->xdgWmBaseGlobals;
}

const std::vector<WpPresentationTime::GWpPresentation *> &LClient::wpPresentationTimeGlobals() const noexcept
{
    return imp()->wpPresentationTimeGlobals;
}

const std::vector<LinuxDMABuf::GLinuxDMABuf *> &LClient::linuxDMABufGlobals() const noexcept
{
    return imp()->linuxDMABufGlobals;
}

const std::vector<FractionalScale::GFractionalScaleManager *> &LClient::fractionalScaleManagerGlobals() const noexcept
{
    return imp()->fractionalScaleManagerGlobals;
}

const std::vector<GammaControl::GGammaControlManager *> &LClient::gammaControlManagerGlobals() const noexcept
{
    return imp()->gammaControlManagerGlobals;
}

const std::vector<TearingControl::GTearingControlManager *> &LClient::tearingControlManagerGlobals() const noexcept
{
    return imp()->tearingControlManagerGlobals;
}

const std::vector<Viewporter::GViewporter *> &LClient::viewporterGlobals() const noexcept
{
    return imp()->viewporterGlobals;
}

const std::vector<XdgDecoration::GXdgDecorationManager *> &LClient::xdgDecorationManagerGlobals() const noexcept
{
    return imp()->xdgDecorationManagerGlobals;
}

const std::vector<RelativePointer::GRelativePointerManager *> &LClient::relativePointerManagerGlobals() const noexcept
{
    return imp()->relativePointerManagerGlobals;
}

const std::vector<PointerGestures::GPointerGestures *> &LClient::pointerGesturesGlobals() const noexcept
{
    return imp()->pointerGesturesGlobals;
}

const LClient::Events &LClient::events() const noexcept
{
    return imp()->events;
}
