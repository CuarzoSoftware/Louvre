#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LClient.h>

LClient::LClient(const void *params) : LPRIVATE_INIT_UNIQUE(LClient)
{
    imp()->client = ((Params*)params)->client;
    dataDevice().imp()->client = this;
}

LClient::~LClient() {}

bool LClient::ping(UInt32 serial) const
{
    if (imp()->xdgWmBaseGlobals.empty())
        return false;

    imp()->xdgWmBaseGlobals.front()->ping(serial);
    return true;
}

wl_client *LClient::client() const
{
    return imp()->client;
}

LDataDevice &LClient::dataDevice() const
{
    return imp()->dataDevice;
}

const std::vector<LSurface *> &LClient::surfaces() const
{
    return imp()->surfaces;
}

void LClient::flush()
{
    wl_client_flush(client());
}

void LClient::destroy()
{
    wl_client_destroy(client());
}

const std::vector<Wayland::GOutput*> &LClient::outputGlobals() const
{
    return imp()->outputGlobals;
}

const std::vector<Wayland::GCompositor*> &LClient::compositorGlobals() const
{
    return imp()->compositorGlobals;
}

const std::vector<Wayland::GSubcompositor*> &LClient::subcompositorGlobals() const
{
    return imp()->subcompositorGlobals;
}

const std::vector<Wayland::GSeat*> &LClient::seatGlobals() const
{
    return imp()->seatGlobals;
}

const Wayland::GDataDeviceManager *LClient::dataDeviceManagerGlobal() const
{
    return imp()->dataDeviceManagerGlobal;
}

const std::vector<XdgShell::GXdgWmBase *> &LClient::xdgWmBaseGlobals() const
{
    return imp()->xdgWmBaseGlobals;
}

const std::vector<WpPresentationTime::GWpPresentation *> &LClient::wpPresentationTimeGlobals() const
{
    return imp()->wpPresentationTimeGlobals;
}

const std::vector<LinuxDMABuf::GLinuxDMABuf *> &LClient::linuxDMABufGlobals() const
{
    return imp()->linuxDMABufGlobals;
}

const std::vector<FractionalScale::GFractionalScaleManager *> &LClient::fractionalScaleManagerGlobals() const
{
    return imp()->fractionalScaleManagerGlobals;
}

const std::vector<GammaControl::GGammaControlManager *> &LClient::gammaControlManagerGlobals() const
{
    return imp()->gammaControlManagerGlobals;
}

const std::vector<TearingControl::GTearingControlManager *> &LClient::tearingControlManagerGlobals() const
{
    return imp()->tearingControlManagerGlobals;
}

const std::vector<Viewporter::GViewporter *> &LClient::viewporterGlobals() const
{
    return imp()->viewporterGlobals;
}

const std::vector<XdgDecoration::GXdgDecorationManager *> &LClient::xdgDecorationManagerGlobals() const
{
    return imp()->xdgDecorationManagerGlobals;
}

const std::vector<RelativePointer::GRelativePointerManager *> &LClient::relativePointerManagerGlobals() const
{
    return imp()->relativePointerManagerGlobals;
}

const LClient::Events &LClient::events() const
{
    return imp()->events;
}
