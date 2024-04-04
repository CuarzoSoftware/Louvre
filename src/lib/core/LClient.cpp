#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LClient.h>

const LEvent *LClient::findEventBySerial(UInt32 serial) const noexcept
{
    if (imp()->events.keyboard.enter.serial() == serial)
        return &imp()->events.keyboard.enter;
    else if (imp()->events.keyboard.leave.serial() == serial)
        return &imp()->events.keyboard.leave;

    for (UInt8 i = 0; i < 5; i++)
        if (imp()->events.keyboard.key[i].serial() == serial)
            return &imp()->events.keyboard.key[i];

    if (imp()->events.keyboard.modifiers.serial() == serial)
        return &imp()->events.keyboard.modifiers;

    else if (imp()->events.pointer.enter.serial() == serial)
        return &imp()->events.pointer.enter;
    else if (imp()->events.pointer.leave.serial() == serial)
        return &imp()->events.pointer.leave;

    for (UInt8 i = 0; i < 5; i++)
        if (imp()->events.pointer.button[i].serial() == serial)
            return &imp()->events.pointer.button[i];

    if (imp()->events.pointer.swipeBegin.serial() == serial)
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

    for (auto *seat : seatGlobals())
        if (seat->dataDeviceRes() && seat->dataDeviceRes()->enterSerial() == serial)
            return &imp()->events.keyboard.enter;

    return nullptr;
}

const LClientCursor &LClient::lastCursorRequest() const noexcept
{
    return imp()->lastCursorRequest;
}

LClient::LClient(const void *params) : m_imp { std::make_unique<LClientPrivate>(this, ((Params*)params)->client)} {}

LClient::~LClient() {}

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

const std::vector<Wayland::GDataDeviceManager*> &LClient::dataDeviceManagerGlobals() const noexcept
{
    return imp()->dataDeviceManagerGlobals;
}

const std::vector<XdgShell::GXdgWmBase *> &LClient::xdgWmBaseGlobals() const noexcept
{
    return imp()->xdgWmBaseGlobals;
}

const std::vector<PresentationTime::GPresentation *> &LClient::presentationTimeGlobals() const noexcept
{
    return imp()->presentationTimeGlobals;
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

const std::vector<SessionLock::GSessionLockManager *> &LClient::sessionLockManagerGlobals() const noexcept
{
    return imp()->sessionLockManagerGlobals;
}

const std::vector<PointerConstraints::GPointerConstraints *> &LClient::pointerConstraintsGlobals() const noexcept
{
    return imp()->pointerConstraintsGlobals;
}

const std::vector<XdgOutput::GXdgOutputManager *> &LClient::xdgOutputManagerGlobals() const noexcept
{
    return imp()->xdgOutputManagerGlobals;
}

const std::vector<ScreenCopy::GScreenCopyManager *> &LClient::screenCopyManagerGlobals() const noexcept
{
    return imp()->screenCopyManagerGlobals;
}

const LClient::Events &LClient::events() const noexcept
{
    return imp()->events;
}
